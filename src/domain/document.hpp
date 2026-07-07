#pragma once

#include "enums.hpp"

#include <string>

namespace dms {

// Tài liệu trong hệ thống.
struct Document {
    std::string     id;
    std::string     ownerUserId;
    std::string     unitScopeId;          // đơn vị sở hữu nếu là UnitPublic, rỗng nếu Personal
    VisibilityType  visibility = VisibilityType::Personal;
    std::string     title;
    std::string     contentPath;          // đường dẫn file nội dung (text)
    std::string     contentType;          // ví dụ: text/plain, text/markdown
    long long       size = 0;             // bytes
    std::string     createdAt;
    std::string     updatedAt;
    bool            isDeleted = false;
};

} // namespace dms
