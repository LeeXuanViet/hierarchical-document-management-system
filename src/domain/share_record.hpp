#pragma once

#include "enums.hpp"

#include <string>

namespace dms {

// Bản ghi chia sẻ tài liệu trực tiếp giữa hai người dùng.
struct ShareRecord {
    std::string      id;
    std::string      documentId;
    std::string      fromUserId;
    std::string      toUserId;
    SharePermission  permission = SharePermission::Read;
};

} // namespace dms
