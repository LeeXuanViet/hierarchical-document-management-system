#pragma once

#include <string>

namespace dms {

// Vai trò người dùng trong hệ thống.
enum class Role {
    User,        // Người dùng thường
    UnitAdmin,   // Quản trị đơn vị
    SysAdmin     // Quản trị hệ thống
};

std::string roleToString(Role r);
Role stringToRole(const std::string& s);

// Kiểu hiển thị của tài liệu.
enum class VisibilityType {
    Personal,     // Tài liệu cá nhân của owner
    UnitPublic,   // Tài liệu công khai của một đơn vị
    DirectShare   // Tài liệu được chia sẻ trực tiếp (ghi nhận qua ShareRecord)
};

std::string visibilityToString(VisibilityType v);
VisibilityType stringToVisibility(const std::string& s);

// Quyền khi chia sẻ tài liệu.
enum class SharePermission {
    Read,
    Edit
};

std::string sharePermissionToString(SharePermission p);
SharePermission stringToSharePermission(const std::string& s);

} // namespace dms
