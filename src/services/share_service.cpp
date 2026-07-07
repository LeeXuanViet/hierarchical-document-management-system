#include "share_service.hpp"

namespace dms {

ShareService::ShareService(IShareRepository& shareRepo,
                           IDocumentRepository& docRepo,
                           IUserRepository& userRepo,
                           IdGenerator& idGen,
                           PermissionService& perms)
    : shareRepo_(shareRepo), docRepo_(docRepo), userRepo_(userRepo),
      idGen_(idGen), perms_(perms) {}

std::string ShareService::share(const std::string& fromUserId,
                                const std::string& documentId,
                                const std::string& toUserId,
                                SharePermission permission) {
    auto fromOpt = userRepo_.findById(fromUserId);
    if (!fromOpt) throw std::invalid_argument("From user not found: " + fromUserId);
    auto toOpt = userRepo_.findById(toUserId);
    if (!toOpt) throw std::invalid_argument("To user not found: " + toUserId);
    if (fromUserId == toUserId) throw std::invalid_argument("Cannot share to yourself");

    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) throw std::invalid_argument("Document not found: " + documentId);

    // Chỉ owner hoặc SysAdmin mới được chia sẻ.
    if (docOpt->ownerUserId != fromUserId && !perms_.isSysAdmin(fromUserId)) {
        throw std::runtime_error("Access denied: only owner can share document " + documentId);
    }

    // Tránh chia sẻ trùng lặp.
    for (const auto& s : shareRepo_.findByFromUser(fromUserId)) {
        if (s.documentId == documentId && s.toUserId == toUserId) {
            // Cập nhật quyền nếu đã tồn tại.
            shareRepo_.remove(s.id);
            break;
        }
    }

    ShareRecord rec;
    rec.id = idGen_.next();
    rec.documentId = documentId;
    rec.fromUserId = fromUserId;
    rec.toUserId = toUserId;
    rec.permission = permission;
    shareRepo_.save(rec);
    return rec.id;
}

void ShareService::revoke(const std::string& fromUserId, const std::string& shareId) {
    // Tìm share record để kiểm tra quyền.
    for (const auto& s : shareRepo_.findByFromUser(fromUserId)) {
        if (s.id == shareId) {
            shareRepo_.remove(shareId);
            return;
        }
    }
    if (perms_.isSysAdmin(fromUserId)) {
        shareRepo_.remove(shareId);
        return;
    }
    throw std::runtime_error("Access denied: cannot revoke share " + shareId);
}

std::vector<ShareRecord> ShareService::listSharedWithMe(const std::string& userId) const {
    return shareRepo_.findByToUser(userId);
}

std::vector<ShareRecord> ShareService::listSharedByMe(const std::string& userId) const {
    return shareRepo_.findByFromUser(userId);
}

} // namespace dms
