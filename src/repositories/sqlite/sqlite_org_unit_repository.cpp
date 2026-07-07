#include "sqlite_org_unit_repository.hpp"

namespace dms {

OrgUnit SqliteOrgUnitRepository::rowToOrgUnit(sqlite3_stmt* stmt) const
{
    OrgUnit u;
    u.id   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* parent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    u.parentUnitId = parent ? parent : "";
    // childrenIds will be populated by findAll() after loading all units
    return u;
}

void SqliteOrgUnitRepository::save(const OrgUnit& unit)
{
    const char* sql = R"(
        INSERT OR REPLACE INTO org_units (id, name, parent_id)
        VALUES (?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, unit.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, unit.name.c_str(), -1, SQLITE_TRANSIENT);
    if (unit.parentUnitId.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, unit.parentUnitId.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<OrgUnit> SqliteOrgUnitRepository::findById(const std::string& id) const
{
    const char* sql = "SELECT id, name, parent_id FROM org_units WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<OrgUnit> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        OrgUnit unit = rowToOrgUnit(stmt);
        sqlite3_finalize(stmt);

        // Populate childrenIds from parent relationships, sorted alphabetically by name
        const char* childSql = "SELECT id FROM org_units WHERE parent_id = ? ORDER BY name;";
        sqlite3_stmt* childStmt = nullptr;
        sqlite3_prepare_v2(db_.handle(), childSql, -1, &childStmt, nullptr);
        sqlite3_bind_text(childStmt, 1, unit.id.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(childStmt) == SQLITE_ROW) {
            unit.childrenIds.push_back(
                reinterpret_cast<const char*>(sqlite3_column_text(childStmt, 0)));
        }
        sqlite3_finalize(childStmt);

        result = std::move(unit);
        return result;
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<OrgUnit> SqliteOrgUnitRepository::findAll() const
{
    const char* sql = "SELECT id, name, parent_id FROM org_units ORDER BY name;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    std::vector<OrgUnit> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToOrgUnit(stmt));
    }
    sqlite3_finalize(stmt);

    // Reconstruct childrenIds from parent relationships (preserving ORDER BY name)
    for (auto& unit : results) {
        unit.childrenIds.clear();
        for (const auto& other : results) {
            if (other.parentUnitId == unit.id) {
                unit.childrenIds.push_back(other.id);
            }
        }
    }
    return results;
}

bool SqliteOrgUnitRepository::remove(const std::string& id)
{
    const char* sql = "DELETE FROM org_units WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changes > 0;
}

} // namespace dms
