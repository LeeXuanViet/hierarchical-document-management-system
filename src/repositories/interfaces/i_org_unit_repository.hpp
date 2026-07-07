#pragma once

#include "domain/org_unit.hpp"

#include <vector>
#include <optional>
#include <string>

namespace dms {

// Interface repository cho đơn vị tổ chức.
class IOrgUnitRepository {
public:
    virtual ~IOrgUnitRepository() = default;

    virtual void save(const OrgUnit& unit) = 0;
    virtual std::optional<OrgUnit> findById(const std::string& id) const = 0;
    virtual std::vector<OrgUnit> findAll() const = 0;
    virtual bool remove(const std::string& id) = 0;
};

} // namespace dms
