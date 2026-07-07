#pragma once

#include <string>
#include <vector>

namespace dms {

// Đơn vị tổ chức dạng cây.
struct OrgUnit {
    std::string id;
    std::string name;
    std::string parentUnitId;   // rỗng nếu là root
    std::vector<std::string> childrenIds;
};

} // namespace dms
