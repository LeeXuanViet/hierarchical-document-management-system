#include "document_service.hpp"

#include <chrono>
#include <sstream>
#include <iomanip>

namespace dms {

namespace {

std::string isoNow() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    ::gmtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

} // namespace

DocumentService::DocumentService(IDocumentRepository& docRepo,
                                 IUserRepository& userRepo,
                                 IFavoriteRepository& favRepo,
                                 FileStore& fileStore,
                                 IdGenerator& idGen,
                                 PermissionService& perms,
                                 UserService& users)
    : docRepo_(docRepo), userRepo_(userRepo), favRepo_(favRepo),
      fileStore_(fileStore), idGen_(idGen), perms_(perms), users_(users) {}

std::string DocumentService::nowIso() const { return isoNow(); }

void DocumentService::enforceRead(const std::string& userId, const std::string& documentId) const {
    if (!perms_.canReadDocument(userId, documentId)) {
        throw std::runtime_error("Access denied: cannot read document " + documentId);
    }
}

void DocumentService::enforceEdit(const std::string& userId, const std::string& documentId) const {
    if (!perms_.canEditDocument(userId, documentId)) {
        throw std::runtime_error("Access denied: cannot edit document " + documentId);
    }
}

void DocumentService::enforceDelete(const std::string& userId, const std::string& documentId) const {
    if (!perms_.canDeleteDocument(userId, documentId)) {
        throw std::runtime_error("Access denied: cannot delete document " + documentId);
    }
}

std::string DocumentService::uploadPersonal(const std::string& userId,
                                            const std::string& title,
                                            const std::string& content) {
    auto userOpt = userRepo_.findById(userId);
    if (!userOpt) throw std::invalid_argument("User not found: " + userId);
    if (title.empty()) throw std::invalid_argument("Title must not be empty");

    // Kiểm tra quota.
    if (userOpt->quotaLimit > 0 &&
        userOpt->quotaUsed + static_cast<long long>(content.size()) > userOpt->quotaLimit) {
        throw std::runtime_error("Quota exceeded for user " + userId);
    }

    Document doc;
    do {
        doc.id = idGen_.next();
    } while (docRepo_.findById(doc.id).has_value());
    doc.ownerUserId = userId;
    doc.unitScopeId = "";
    doc.visibility = VisibilityType::Personal;
    doc.title = title;
    doc.contentType = "text/plain";
    doc.size = static_cast<long long>(content.size());
    doc.createdAt = nowIso();
    doc.updatedAt = doc.createdAt;
    doc.isDeleted = false;

    doc.contentPath = fileStore_.write(doc.id, content);
    if (doc.contentPath.empty()) {
        throw std::runtime_error("Failed to write document content to disk");
    }

    docRepo_.save(doc);
    users_.adjustQuotaUsed(userId, static_cast<long long>(content.size()));
    return doc.id;
}

std::string DocumentService::uploadUnitPublic(const std::string& userId,
                                              const std::string& unitId,
                                              const std::string& title,
                                              const std::string& content) {
    if (!perms_.canUploadUnitPublic(userId, unitId)) {
        throw std::runtime_error("Access denied: cannot upload public document to unit " + unitId);
    }
    if (title.empty()) throw std::invalid_argument("Title must not be empty");

    Document doc;
    do {
        doc.id = idGen_.next();
    } while (docRepo_.findById(doc.id).has_value());
    doc.ownerUserId = userId;
    doc.unitScopeId = unitId;
    doc.visibility = VisibilityType::UnitPublic;
    doc.title = title;
    doc.contentType = "text/plain";
    doc.size = static_cast<long long>(content.size());
    doc.createdAt = nowIso();
    doc.updatedAt = doc.createdAt;
    doc.isDeleted = false;

    doc.contentPath = fileStore_.write(doc.id, content);
    if (doc.contentPath.empty()) {
        throw std::runtime_error("Failed to write document content to disk");
    }

    docRepo_.save(doc);
    // Tài liệu công khai đơn vị không tính vào quota cá nhân (chính sách demo).
    return doc.id;
}

void DocumentService::edit(const std::string& userId,
                           const std::string& documentId,
                           const std::string& newTitle,
                           const std::string& newContent) {
    enforceEdit(userId, documentId);
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) throw std::invalid_argument("Document not found: " + documentId);

    long long oldSize = docOpt->size;
    long long newSize = static_cast<long long>(newContent.size());

    // Kiểm tra quota nếu là tài liệu cá nhân và owner tự sửa.
    if (docOpt->visibility == VisibilityType::Personal && docOpt->ownerUserId == userId) {
        auto userOpt = userRepo_.findById(userId);
        if (userOpt && userOpt->quotaLimit > 0) {
            long long projected = userOpt->quotaUsed - oldSize + newSize;
            if (projected > userOpt->quotaLimit) {
                throw std::runtime_error("Quota exceeded when editing document " + documentId);
            }
        }
    }

    if (!newTitle.empty()) docOpt->title = newTitle;
    docOpt->size = newSize;
    docOpt->updatedAt = nowIso();

    // Ghi lại nội dung mới.
    if (!fileStore_.write(documentId, newContent).empty()) {
        docOpt->contentPath = documentId + ".txt";
    }
    docRepo_.save(*docOpt);

    // Cập nhật quota cho owner nếu là tài liệu cá nhân.
    if (docOpt->visibility == VisibilityType::Personal) {
        users_.adjustQuotaUsed(docOpt->ownerUserId, newSize - oldSize);
    }
}

void DocumentService::remove(const std::string& userId, const std::string& documentId) {
    enforceDelete(userId, documentId);
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) throw std::invalid_argument("Document not found: " + documentId);

    long long size = docOpt->size;
    std::string owner = docOpt->ownerUserId;
    bool personal = (docOpt->visibility == VisibilityType::Personal);

    fileStore_.remove(docOpt->contentPath);
    docRepo_.remove(documentId);

    if (personal) {
        users_.adjustQuotaUsed(owner, -size);
    }
}

std::string DocumentService::readContent(const std::string& userId, const std::string& documentId) const {
    enforceRead(userId, documentId);
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) throw std::invalid_argument("Document not found: " + documentId);
    return fileStore_.read(docOpt->contentPath);
}

Document DocumentService::getDocument(const std::string& userId, const std::string& documentId) const {
    enforceRead(userId, documentId);
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) throw std::invalid_argument("Document not found: " + documentId);
    return *docOpt;
}

std::vector<Document> DocumentService::listMine(const std::string& userId) const {
    return docRepo_.findByOwner(userId);
}

std::vector<Document> DocumentService::listVisibleUnitPublic(const std::string& userId) const {
    auto userOpt = userRepo_.findById(userId);
    if (!userOpt) return {};
    std::vector<Document> result;
    for (const auto& d : docRepo_.findAll()) {
        if (d.visibility != VisibilityType::UnitPublic) continue;
        if (d.unitScopeId.empty()) continue;
        // Hiển thị cho user thuộc unitScope hoặc descendant của unitScope.
        // Tức là unitScope phải là tổ tiên (hoặc chính) của unit của user.
        // Dùng OrgTreeService thông qua permission: canReadDocument.
        if (perms_.canReadDocument(userId, d.id)) {
            result.push_back(d);
        }
    }
    return result;
}

void DocumentService::addFavorite(const std::string& userId, const std::string& documentId) {
    enforceRead(userId, documentId);
    favRepo_.add(FavoriteRecord{userId, documentId});
}

void DocumentService::removeFavorite(const std::string& userId, const std::string& documentId) {
    favRepo_.remove(userId, documentId);
}

std::vector<Document> DocumentService::listFavorites(const std::string& userId) const {
    std::vector<Document> result;
    for (const auto& f : favRepo_.findByUser(userId)) {
        auto d = docRepo_.findById(f.documentId);
        if (d) result.push_back(*d);
    }
    return result;
}

std::string DocumentService::preview(const std::string& userId, const std::string& documentId, size_t maxChars) const {
    std::string content = readContent(userId, documentId);
    if (content.size() <= maxChars) return content;
    return content.substr(0, maxChars) + "...";
}

} // namespace dms
