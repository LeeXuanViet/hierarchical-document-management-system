#pragma once

#include "../repositories/interfaces/i_share_repository.hpp"
#include "../repositories/interfaces/i_document_repository.hpp"
#include "../repositories/interfaces/i_user_repository.hpp"
#include "../utils/id_generator.hpp"
#include "permission_service.hpp"

#include <vector>
#include <string>
#include <stdexcept>

namespace dms {

// Service chia sẻ tài liệu trực tiếp giữa hai người dùng.
class ShareService {
public:
    ShareService(IShareRepository& shareRepo,
                 IDocumentRepository& docRepo,
                 IUserRepository& userRepo,
                 IdGenerator& idGen,
                 PermissionService& perms);

    // Chia sẻ tài liệu từ fromUserId sang toUserId với quyền Read/Edit.
    // Chỉ owner (hoặc SysAdmin) mới được chia sẻ tài liệu cá nhân của mình.
    std::string share(const std::string& fromUserId,
                      const std::string& documentId,
                      const std::string& toUserId,
                      SharePermission permission);

    // Hủy chia sẻ.
    void revoke(const std::string& fromUserId, const std::string& shareId);

    // Liệt kê tài liệu được chia sẻ cho user.
    std::vector<ShareRecord> listSharedWithMe(const std::string& userId) const;

    // Liệt kê tài liệu user đã chia sẻ cho người khác.
    std::vector<ShareRecord> listSharedByMe(const std::string& userId) const;

private:
    IShareRepository& shareRepo_;
    IDocumentRepository& docRepo_;
    IUserRepository& userRepo_;
    IdGenerator& idGen_;
    PermissionService& perms_;
};

} // namespace dms
