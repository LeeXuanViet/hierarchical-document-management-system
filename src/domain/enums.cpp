#include "enums.hpp"

#include <stdexcept>

namespace dms {

std::string roleToString(Role r) {
    switch (r) {
        case Role::User:       return "USER";
        case Role::UnitAdmin:  return "UNIT_ADMIN";
        case Role::SysAdmin:   return "SYS_ADMIN";
    }
    throw std::invalid_argument("Unknown role");
}

Role stringToRole(const std::string& s) {
    if (s == "USER")        return Role::User;
    if (s == "UNIT_ADMIN")  return Role::UnitAdmin;
    if (s == "SYS_ADMIN")   return Role::SysAdmin;
    throw std::invalid_argument("Unknown role string: " + s);
}

std::string visibilityToString(VisibilityType v) {
    switch (v) {
        case VisibilityType::Personal:     return "PERSONAL";
        case VisibilityType::UnitPublic:   return "UNIT_PUBLIC";
        case VisibilityType::DirectShare:  return "DIRECT_SHARE";
    }
    throw std::invalid_argument("Unknown visibility");
}

VisibilityType stringToVisibility(const std::string& s) {
    if (s == "PERSONAL")      return VisibilityType::Personal;
    if (s == "UNIT_PUBLIC")   return VisibilityType::UnitPublic;
    if (s == "DIRECT_SHARE")  return VisibilityType::DirectShare;
    throw std::invalid_argument("Unknown visibility string: " + s);
}

std::string sharePermissionToString(SharePermission p) {
    switch (p) {
        case SharePermission::Read: return "READ";
        case SharePermission::Edit: return "EDIT";
    }
    throw std::invalid_argument("Unknown share permission");
}

SharePermission stringToSharePermission(const std::string& s) {
    if (s == "READ") return SharePermission::Read;
    if (s == "EDIT") return SharePermission::Edit;
    throw std::invalid_argument("Unknown share permission string: " + s);
}

} // namespace dms
