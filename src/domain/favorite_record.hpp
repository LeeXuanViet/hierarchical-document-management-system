#pragma once

#include <string>

namespace dms {

// Bản ghi đánh dấu tài liệu quan trọng (favorite) của một người dùng.
struct FavoriteRecord {
    std::string userId;
    std::string documentId;
};

} // namespace dms
