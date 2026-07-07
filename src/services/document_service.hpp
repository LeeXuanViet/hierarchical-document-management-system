#pragma once

#include "../repositories/interfaces/i_document_repository.hpp"
#include "../repositories/interfaces/i_user_repository.hpp"
#include "../repositories/interfaces/i_favorite_repository.hpp"
#include "../storage/file_store.hpp"
#include "../utils/id_generator.hpp"
#include "permission_service.hpp"
#include "user_service.hpp"

#include <vector>
#include <string>
#include <stdexcept>

namespace dms {

// Service quản lý tài liệu: upload, sửa, xóa, favorite, liệt kê, preview.
class DocumentService {
public:
    DocumentService(IDocumentRepository& docRepo,
                    IUserRepository& userRepo,
                    IFavoriteRepository& favRepo,
                    FileStore& fileStore,
                    IdGenerator& idGen,
                    PermissionService& perms,
                    UserService& users);

    // Upload tài liệu cá nhân. Trả về id tài liệu.
    std::string uploadPersonal(const std::string& userId,
                               const std::string& title,
                               const std::string& content);

    // Upload tài liệu công khai đơn vị (UnitAdmin của unitId mới được gọi).
    std::string uploadUnitPublic(const std::string& userId,
                                 const std::string& unitId,
                                 const std::string& title,
                                 const std::string& content);

    // Sửa nội dung + tiêu đề tài liệu (cần quyền edit).
    void edit(const std::string& userId,
              const std::string& documentId,
              const std::string& newTitle,
              const std::string& newContent);

    // Xóa tài liệu (cần quyền delete).
    void remove(const std::string& userId, const std::string& documentId);

    // Đọc nội dung tài liệu (cần quyền read). Trả về nội dung text.
    std::string readContent(const std::string& userId, const std::string& documentId) const;

    // Lấy metadata tài liệu (cần quyền read).
    Document getDocument(const std::string& userId, const std::string& documentId) const;

    // Liệt kê tài liệu cá nhân của user.
    std::vector<Document> listMine(const std::string& userId) const;

    // Liệt kê tài liệu công khai mà user có thể thấy (từ đơn vị của user và tổ tiên).
    std::vector<Document> listVisibleUnitPublic(const std::string& userId) const;

    // Đánh dấu favorite.
    void addFavorite(const std::string& userId, const std::string& documentId);

    // Bỏ favorite.
    void removeFavorite(const std::string& userId, const std::string& documentId);

    // Liệt kê favorite của user.
    std::vector<Document> listFavorites(const std::string& userId) const;

    // Preview nội dung (giới hạn số ký tự).
    std::string preview(const std::string& userId, const std::string& documentId, size_t maxChars = 200) const;

private:
    IDocumentRepository& docRepo_;
    IUserRepository& userRepo_;
    IFavoriteRepository& favRepo_;
    FileStore& fileStore_;
    IdGenerator& idGen_;
    PermissionService& perms_;
    UserService& users_;

    std::string nowIso() const;
    void enforceRead(const std::string& userId, const std::string& documentId) const;
    void enforceEdit(const std::string& userId, const std::string& documentId) const;
    void enforceDelete(const std::string& userId, const std::string& documentId) const;
};

} // namespace dms
