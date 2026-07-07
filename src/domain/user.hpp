#pragma once

#include "enums.hpp"

#include <string>

namespace dms {

// Người dùng thuộc về một đơn vị cụ thể.
struct User {
    std::string id;
    std::string username;
    std::string passwordHash;
    Role        role = Role::User;
    std::string unitId;
    long long   quotaLimit = 0;   // bytes, 0 = không giới hạn
    long long   quotaUsed  = 0;   // bytes
};

} // namespace dms
