#pragma once

#include "../repositories/interfaces/i_document_repository.hpp"
#include "../storage/file_store.hpp"
#include "permission_service.hpp"
#include "../utils/tokenizer.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace dms {

// Service tìm kiếm tài liệu theo tên hoặc nội dung.
// Dùng inverted index cho nội dung text để tăng hiệu năng.
class SearchService {
public:
    // Callback: given (title, rawContent) -> extracted plain text.
    // Used to extract searchable text from binary formats (e.g. PDF) before
    // tokenizing. If not set, the raw content is tokenized directly.
    using TextExtractor = std::function<std::string(const std::string& title,
                                                    const std::string& rawContent)>;

    SearchService(IDocumentRepository& docRepo,
                  FileStore& fileStore,
                  PermissionService& perms);

    // Set a custom text extractor for binary formats (e.g. PDF). When set,
    // indexDocument() calls it to obtain plain text before tokenizing.
    void setTextExtractor(TextExtractor extractor);

    // Xây dựng lại inverted index từ toàn bộ tài liệu text.
    void rebuildIndex();

    // Thêm/cập nhật một tài liệu vào index.
    void indexDocument(const std::string& documentId);

    // Xóa một tài liệu khỏi index.
    void removeFromIndex(const std::string& documentId);

    // Tìm theo tiêu đề (substring, không phân biệt hoa thường).
    // Chỉ trả về tài liệu user có quyền đọc.
    std::vector<Document> searchByTitle(const std::string& userId, const std::string& query) const;

    // Tìm theo nội dung (inverted index, AND tất cả token).
    // Chỉ trả về tài liệu user có quyền đọc.
    std::vector<Document> searchByContent(const std::string& userId, const std::string& query) const;

private:
    IDocumentRepository& docRepo_;
    FileStore& fileStore_;
    PermissionService& perms_;
    TextExtractor textExtractor_;  // optional; extracts text from binary formats

    // token -> set documentId
    std::unordered_map<std::string, std::unordered_set<std::string>> index_;
    // documentId -> set token (để dễ xóa)
    std::unordered_map<std::string, std::unordered_set<std::string>> docTokens_;
};

} // namespace dms
