#pragma once

#include "../repositories/interfaces/i_org_unit_repository.hpp"
#include "../utils/id_generator.hpp"

#include <vector>
#include <string>
#include <stdexcept>

namespace dms {

// Service quản lý cây đơn vị tổ chức.
// Cung cấp tạo đơn vị, duyệt cây, kiểm tra quan hệ ancestor/descendant.
class OrgTreeService {
public:
    OrgTreeService(IOrgUnitRepository& repo, IdGenerator& idGen);

    // Tạo đơn vị mới. parentUnitId rỗng nghĩa là root.
    // Trả về id đơn vị mới tạo.
    std::string createUnit(const std::string& name, const std::string& parentUnitId);

    // Lấy toàn bộ cây dưới dạng danh sách (đã sắp xếp theo DFS từ root).
    std::vector<OrgUnit> getTree() const;

    // Lấy cây con của một đơn vị (bao gồm chính nó).
    std::vector<OrgUnit> getSubtree(const std::string& unitId) const;

    // Lấy danh sách tổ tiên của một đơn vị (từ gần nhất đến root).
    std::vector<OrgUnit> getAncestors(const std::string& unitId) const;

    // Kiểm tra unitId có phải là tổ tiên (hoặc chính) của descendantId hay không.
    bool isAncestorOrSelf(const std::string& unitId, const std::string& descendantId) const;

    // Kiểm tra unitId có tồn tại.
    bool exists(const std::string& unitId) const;

    // Lấy thông tin đơn vị.
    OrgUnit getUnit(const std::string& unitId) const;

private:
    IOrgUnitRepository& repo_;
    IdGenerator& idGen_;
};

} // namespace dms
