#include "search_service.hpp"

#include <algorithm>

namespace dms {

SearchService::SearchService(IDocumentRepository& docRepo,
                             FileStore& fileStore,
                             PermissionService& perms)
    : docRepo_(docRepo), fileStore_(fileStore), perms_(perms) {}

void SearchService::setTextExtractor(TextExtractor extractor) {
    textExtractor_ = std::move(extractor);
}

void SearchService::rebuildIndex() {
    index_.clear();
    docTokens_.clear();
    for (const auto& d : docRepo_.findAll()) {
        indexDocument(d.id);
    }
}

void SearchService::indexDocument(const std::string& documentId) {
    // Xóa entry cũ nếu có.
    removeFromIndex(documentId);

    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) return;

    std::string content = fileStore_.read(docOpt->contentPath);

    // Detect binary content (null bytes in first 1024 bytes). For binary
    // files (e.g. PDF), use the text extractor callback if available to
    // obtain plain text before tokenizing. This avoids indexing garbage
    // tokens from raw binary data.
    bool isBinary = false;
    size_t checkLen = std::min(content.size(), static_cast<size_t>(1024));
    for (size_t i = 0; i < checkLen; ++i) {
        if (content[i] == '\0') { isBinary = true; break; }
    }

    std::string textToIndex;
    if (isBinary && textExtractor_) {
        textToIndex = textExtractor_(docOpt->title, content);
    } else if (isBinary) {
        // Binary file with no extractor: skip indexing (no meaningful tokens).
        docTokens_[documentId] = {};
        return;
    } else {
        textToIndex = content;
    }

    auto tokens = tokenize(textToIndex);
    std::unordered_set<std::string> uniqueTokens(tokens.begin(), tokens.end());

    for (const auto& t : uniqueTokens) {
        index_[t].insert(documentId);
    }
    docTokens_[documentId] = std::move(uniqueTokens);
}

void SearchService::removeFromIndex(const std::string& documentId) {
    auto it = docTokens_.find(documentId);
    if (it == docTokens_.end()) return;
    for (const auto& t : it->second) {
        auto idxIt = index_.find(t);
        if (idxIt != index_.end()) {
            idxIt->second.erase(documentId);
            if (idxIt->second.empty()) index_.erase(idxIt);
        }
    }
    docTokens_.erase(it);
}

std::vector<Document> SearchService::searchByTitle(const std::string& userId, const std::string& query) const {
    std::vector<Document> result;
    for (const auto& d : docRepo_.findAll()) {
        if (!perms_.canReadDocument(userId, d.id)) continue;
        if (containsIgnoreCase(d.title, query)) {
            result.push_back(d);
        }
    }
    return result;
}

std::vector<Document> SearchService::searchByContent(const std::string& userId, const std::string& query) const {
    auto queryTokens = tokenize(query);
    if (queryTokens.empty()) return {};

    // Lấy giao của các tập documentId cho từng token (AND logic).
    std::unordered_set<std::string> candidates;
    bool first = true;
    for (const auto& t : queryTokens) {
        auto it = index_.find(t);
        if (it == index_.end()) return {}; // một token không có -> rỗng
        if (first) {
            candidates = it->second;
            first = false;
        } else {
            std::unordered_set<std::string> intersection;
            for (const auto& id : candidates) {
                if (it->second.count(id)) intersection.insert(id);
            }
            candidates = std::move(intersection);
        }
        if (candidates.empty()) return {};
    }

    std::vector<Document> result;
    for (const auto& id : candidates) {
        if (!perms_.canReadDocument(userId, id)) continue;
        auto d = docRepo_.findById(id);
        if (d) result.push_back(*d);
    }
    return result;
}

} // namespace dms
