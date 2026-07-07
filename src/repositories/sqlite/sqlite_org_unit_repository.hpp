#pragma once

#include "sqlite_database.hpp"
#include "repositories/interfaces/i_org_unit_repository.hpp"

namespace dms {

class SqliteOrgUnitRepository : public IOrgUnitRepository {
public:
    explicit SqliteOrgUnitRepository(SqliteDatabase& db) : db_(db) {}

    void save(const OrgUnit& unit) override;
    std::optional<OrgUnit> findById(const std::string& id) const override;
    std::vector<OrgUnit> findAll() const override;
    bool remove(const std::string& id) override;

private:
    OrgUnit rowToOrgUnit(struct sqlite3_stmt* stmt) const;
    SqliteDatabase& db_;
};

} // namespace dms
