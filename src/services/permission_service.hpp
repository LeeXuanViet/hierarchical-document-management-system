#pragma once

#include "../repositories/interfaces/i_user_repository.hpp"
#include "../repositories/interfaces/i_document_repository.hpp"
#include "../repositories/interfaces/i_share_repository.hpp"
#include "org_tree_service.hpp"

#include <string>

namespace dms {

// Engine phân quyền tập trung.
// Mọi kiểm tra truy cập tài liệu/đơn vị đều đi qua service này.
class PermissionService {
public:
    PermissionService(IUserRepository& userRepo,
                      IDocumentRepository& docRepo,
                      IShareRepository& shareRepo,
                      OrgTreeService& orgTree);

    // Kiểm tra user có thể ĐỌC tài liệu hay không.
    bool canReadDocument(const std::string& userId, const std::string& documentId) const;

    // Kiểm tra user có thể SỬA tài liệu hay không.
    bool canEditDocument(const std::string& userId, const std::string& documentId) const;

    // Kiểm tra user có thể XÓA tài liệu hay không.
    bool canDeleteDocument(const std::string& userId, const std::string& documentId) const;

    // Kiểm tra user có thể upload tài liệu công khai vào đơn vị unitId hay không.
    // Quy tắc: phải là UnitAdmin của chính đơn vị đó, hoặc SysAdmin.
    bool canUploadUnitPublic(const std::string& userId, const std::string& unitId) const;

    // Kiểm tra user có quyền quản trị hệ thống.
    bool isSysAdmin(const std::string& userId) const;

    // Kiểm tra user có quyền quản trị đơn vị unitId.
    bool isUnitAdminOf(const std::string& userId, const std::string& unitId) const;

    // Giải thích lý do user có thể đọc tài liệu (dùng cho demo/audit).
    // Trả về chuỗi mô tả: OWNER / SHARED / UNIT_PUBLIC / SYS_ADMIN / DENIED.
    std::string explainReadAccess(const std::string& userId, const std::string& documentId) const;

private:
    IUserRepository& userRepo_;
    IDocumentRepository& docRepo_;
    IShareRepository& shareRepo_;
    OrgTreeService& orgTree_;
};

} // namespace dms
