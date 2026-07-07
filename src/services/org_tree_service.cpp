#include "org_tree_service.hpp"

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace dms {

OrgTreeService::OrgTreeService(IOrgUnitRepository& repo, IdGenerator& idGen)
    : repo_(repo), idGen_(idGen) {}

std::string OrgTreeService::createUnit(const std::string& name, const std::string& parentUnitId) {
    if (name.empty()) {
        throw std::invalid_argument("Unit name must not be empty");
    }
    if (!parentUnitId.empty() && !exists(parentUnitId)) {
        throw std::invalid_argument("Parent unit does not exist: " + parentUnitId);
    }

    OrgUnit unit;
    unit.id = idGen_.next();
    unit.name = name;
    unit.parentUnitId = parentUnitId;

    repo_.save(unit);

    // Cập nhật childrenIds của parent.
    if (!parentUnitId.empty()) {
        auto parentOpt = repo_.findById(parentUnitId);
        if (parentOpt) {
            parentOpt->childrenIds.push_back(unit.id);
            repo_.save(*parentOpt);
        }
    }
    return unit.id;
}

std::vector<OrgUnit> OrgTreeService::getTree() const {
    auto all = repo_.findAll();
    // Sắp xếp DFS từ các root.
    std::unordered_map<std::string, OrgUnit> byId;
    std::vector<std::string> roots;
    for (const auto& u : all) {
        byId[u.id] = u;
        if (u.parentUnitId.empty()) roots.push_back(u.id);
    }
    std::vector<OrgUnit> result;
    std::function<void(const std::string&)> dfs = [&](const std::string& id) {
        auto it = byId.find(id);
        if (it == byId.end()) return;
        result.push_back(it->second);
        for (const auto& cid : it->second.childrenIds) dfs(cid);
    };
    for (const auto& r : roots) dfs(r);
    return result;
}

std::vector<OrgUnit> OrgTreeService::getSubtree(const std::string& unitId) const {
    if (!exists(unitId)) return {};
    auto all = repo_.findAll();
    std::unordered_map<std::string, OrgUnit> byId;
    for (const auto& u : all) byId[u.id] = u;

    std::vector<OrgUnit> result;
    std::function<void(const std::string&)> dfs = [&](const std::string& id) {
        auto it = byId.find(id);
        if (it == byId.end()) return;
        result.push_back(it->second);
        for (const auto& cid : it->second.childrenIds) dfs(cid);
    };
    dfs(unitId);
    return result;
}

std::vector<OrgUnit> OrgTreeService::getAncestors(const std::string& unitId) const {
    std::vector<OrgUnit> result;
    std::string current = unitId;
    while (true) {
        auto opt = repo_.findById(current);
        if (!opt) break;
        if (opt->parentUnitId.empty()) break;
        auto parent = repo_.findById(opt->parentUnitId);
        if (!parent) break;
        result.push_back(*parent);
        current = parent->id;
    }
    return result;
}

bool OrgTreeService::isAncestorOrSelf(const std::string& unitId, const std::string& descendantId) const {
    if (unitId == descendantId) return true;
    std::string current = descendantId;
    while (true) {
        auto opt = repo_.findById(current);
        if (!opt || opt->parentUnitId.empty()) return false;
        if (opt->parentUnitId == unitId) return true;
        current = opt->parentUnitId;
    }
}

bool OrgTreeService::exists(const std::string& unitId) const {
    return repo_.findById(unitId).has_value();
}

OrgUnit OrgTreeService::getUnit(const std::string& unitId) const {
    auto opt = repo_.findById(unitId);
    if (!opt) throw std::invalid_argument("Unit not found: " + unitId);
    return *opt;
}

} // namespace dms
