#include "permission_service.hpp"

namespace dms {

PermissionService::PermissionService(IUserRepository& userRepo,
                                     IDocumentRepository& docRepo,
                                     IShareRepository& shareRepo,
                                     OrgTreeService& orgTree)
    : userRepo_(userRepo), docRepo_(docRepo), shareRepo_(shareRepo), orgTree_(orgTree) {}

bool PermissionService::isSysAdmin(const std::string& userId) const {
    auto u = userRepo_.findById(userId);
    return u.has_value() && u->role == Role::SysAdmin;
}

bool PermissionService::isUnitAdminOf(const std::string& userId, const std::string& unitId) const {
    auto u = userRepo_.findById(userId);
    if (!u) return false;
    if (u->role == Role::SysAdmin) return true;
    if (u->role != Role::UnitAdmin) return false;
    return u->unitId == unitId;
}

bool PermissionService::canUploadUnitPublic(const std::string& userId, const std::string& unitId) const {
    return isUnitAdminOf(userId, unitId);
}

std::string PermissionService::explainReadAccess(const std::string& userId, const std::string& documentId) const {
    auto userOpt = userRepo_.findById(userId);
    if (!userOpt) return "DENIED";
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) return "DENIED";

    if (userOpt->role == Role::SysAdmin) return "SYS_ADMIN";
    if (docOpt->ownerUserId == userId) return "OWNER";

    // Kiểm tra chia sẻ trực tiếp.
    for (const auto& s : shareRepo_.findByToUser(userId)) {
        if (s.documentId == documentId) return "SHARED";
    }

    // Kiểm tra tài liệu công khai của đơn vị tổ tiên (hoặc chính đơn vị).
    if (docOpt->visibility == VisibilityType::UnitPublic && !docOpt->unitScopeId.empty()) {
        if (orgTree_.isAncestorOrSelf(docOpt->unitScopeId, userOpt->unitId)) {
            return "UNIT_PUBLIC";
        }
    }
    return "DENIED";
}

bool PermissionService::canReadDocument(const std::string& userId, const std::string& documentId) const {
    return explainReadAccess(userId, documentId) != "DENIED";
}

bool PermissionService::canEditDocument(const std::string& userId, const std::string& documentId) const {
    auto userOpt = userRepo_.findById(userId);
    if (!userOpt) return false;
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) return false;

    if (userOpt->role == Role::SysAdmin) return true;
    if (docOpt->ownerUserId == userId) return true;

    // Tài liệu công khai đơn vị: chỉ UnitAdmin của đơn vị sở hữu mới sửa.
    if (docOpt->visibility == VisibilityType::UnitPublic) {
        return isUnitAdminOf(userId, docOpt->unitScopeId);
    }

    // Tài liệu cá nhân được chia sẻ với quyền EDIT.
    for (const auto& s : shareRepo_.findByToUser(userId)) {
        if (s.documentId == documentId && s.permission == SharePermission::Edit) {
            return true;
        }
    }
    return false;
}

bool PermissionService::canDeleteDocument(const std::string& userId, const std::string& documentId) const {
    auto userOpt = userRepo_.findById(userId);
    if (!userOpt) return false;
    auto docOpt = docRepo_.findById(documentId);
    if (!docOpt) return false;

    if (userOpt->role == Role::SysAdmin) return true;
    if (docOpt->ownerUserId == userId) return true;

    // Tài liệu công khai đơn vị: UnitAdmin của đơn vị sở hữu có thể xóa.
    if (docOpt->visibility == VisibilityType::UnitPublic) {
        return isUnitAdminOf(userId, docOpt->unitScopeId);
    }
    return false;
}

} // namespace dms
