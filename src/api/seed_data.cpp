#include "seed_data.hpp"

#include <iostream>

namespace dms {

void seedDemo(AppContext& app) {
    using namespace dms;

    // Tạo cây đơn vị: HQ -> DivisionA -> {TeamA1, TeamA2}, DivisionB
    std::string hq = app.orgTree().createUnit("Headquarters", "");
    std::string divA = app.orgTree().createUnit("DivisionA", hq);
    std::string divB = app.orgTree().createUnit("DivisionB", hq);
    std::string teamA1 = app.orgTree().createUnit("TeamA1", divA);
    std::string teamA2 = app.orgTree().createUnit("TeamA2", divA);

    // Tạo người dùng
    std::string admin = app.users().createUser("admin", "admin123", Role::SysAdmin, hq, 0);
    std::string unitAdminA = app.users().createUser("leadA", "lead123", Role::UnitAdmin, divA, 0);
    std::string uA1 = app.users().createUser("alice", "alice123", Role::User, teamA1, 1024 * 1024);
    std::string uA2 = app.users().createUser("bob", "bob123", Role::User, teamA2, 1024 * 1024);
    std::string uB = app.users().createUser("carol", "carol123", Role::User, divB, 1024 * 1024);

    // Tài liệu công khai của DivisionA (do leadA - UnitAdmin upload)
    app.docs().uploadUnitPublic(unitAdminA, divA,
        "DivisionA Policy",
        "This is the public policy of DivisionA. All teams under DivisionA must follow these guidelines.");

    // Tài liệu cá nhân của alice
    app.docs().uploadPersonal(uA1, "Alice Notes",
        "Alice personal notes about the project. Contains keywords: architecture, permission, tree, hierarchy.");

    // Xây index tìm kiếm
    app.search().rebuildIndex();

    std::cout << "=== Demo data seeded ===\n";
    std::cout << "Units:  HQ=" << hq << " DivisionA=" << divA << " DivisionB=" << divB
              << " TeamA1=" << teamA1 << " TeamA2=" << teamA2 << "\n";
    std::cout << "Users:  admin=" << admin << " leadA=" << unitAdminA
              << " alice=" << uA1 << " bob=" << uA2 << " carol=" << uB << "\n";
    std::cout << "Type 'help' for command list.\n\n";
}

} // namespace dms
