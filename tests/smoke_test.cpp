// Smoke test: kiểm tra các kịch bản phân quyền chính.
// Không dùng framework ngoài, chỉ assert thủ công và in kết quả.
#include "api/app_context.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>

namespace {

int failures = 0;
int passes = 0;

void check(bool cond, const std::string& name) {
    if (cond) {
        ++passes;
        std::cout << "[PASS] " << name << "\n";
    } else {
        ++failures;
        std::cout << "[FAIL] " << name << "\n";
    }
}

} // namespace

int main() {
    using namespace dms;
    // Use a separate test database; remove it first to ensure fresh state.
    std::remove("data/test_dms.db");
    AppContext app("data/test_dms.db", "data/test_files");

    // Cây: HQ -> DivisionA -> {TeamA1, TeamA2}, DivisionB
    std::string hq = app.orgTree().createUnit("HQ", "");
    std::string divA = app.orgTree().createUnit("DivisionA", hq);
    std::string divB = app.orgTree().createUnit("DivisionB", hq);
    std::string teamA1 = app.orgTree().createUnit("TeamA1", divA);
    std::string teamA2 = app.orgTree().createUnit("TeamA2", divA);

    std::string admin = app.users().createUser("admin", "p", Role::SysAdmin, hq);
    std::string leadA = app.users().createUser("leadA", "p", Role::UnitAdmin, divA);
    std::string alice = app.users().createUser("alice", "p", Role::User, teamA1);
    std::string bob = app.users().createUser("bob", "p", Role::User, teamA2);
    std::string carol = app.users().createUser("carol", "p", Role::User, divB);

    // 1. Quan hệ ancestor/descendant
    check(app.orgTree().isAncestorOrSelf(divA, teamA1), "DivisionA is ancestor of TeamA1");
    check(app.orgTree().isAncestorOrSelf(divA, teamA2), "DivisionA is ancestor of TeamA2");
    check(!app.orgTree().isAncestorOrSelf(divA, divB), "DivisionA is NOT ancestor of DivisionB");
    check(app.orgTree().isAncestorOrSelf(hq, teamA1), "HQ is ancestor of TeamA1");

    // 2. UnitAdmin upload public vào đơn vị của mình
    std::string pubDoc = app.docs().uploadUnitPublic(leadA, divA, "Policy", "content policy");
    check(!pubDoc.empty(), "UnitAdmin can upload public to own unit");

    // 3. User thường không upload public được
    bool threw = false;
    try { app.docs().uploadUnitPublic(alice, divA, "x", "y"); }
    catch (...) { threw = true; }
    check(threw, "Normal user cannot upload unit-public");

    // 4. Alice (TeamA1) thấy được public của DivisionA (tổ tiên)
    check(app.perms().canReadDocument(alice, pubDoc), "Alice can read DivisionA public (descendant)");
    // 5. Bob (TeamA2) cũng thấy
    check(app.perms().canReadDocument(bob, pubDoc), "Bob can read DivisionA public (descendant)");
    // 6. Carol (DivisionB) KHÔNG thấy
    check(!app.perms().canReadDocument(carol, pubDoc), "Carol cannot read DivisionA public (sibling)");

    // 7. Explain access
    check(app.perms().explainReadAccess(alice, pubDoc) == "UNIT_PUBLIC", "Alice access reason = UNIT_PUBLIC");
    check(app.perms().explainReadAccess(carol, pubDoc) == "DENIED", "Carol access reason = DENIED");

    // 8. Tài liệu cá nhân: chỉ owner đọc được
    std::string aliceDoc = app.docs().uploadPersonal(alice, "Alice Notes", "secret notes");
    check(app.perms().canReadDocument(alice, aliceDoc), "Owner can read own personal doc");
    check(!app.perms().canReadDocument(bob, aliceDoc), "Other user cannot read personal doc");

    // 9. Chia sẻ trực tiếp
    std::string shareId = app.shares().share(alice, aliceDoc, bob, SharePermission::Read);
    check(!shareId.empty(), "Alice can share her doc to Bob");
    check(app.perms().canReadDocument(bob, aliceDoc), "Bob can read shared doc");
    check(!app.perms().canEditDocument(bob, aliceDoc), "Bob cannot edit READ-shared doc");

    // 10. Chia sẻ EDIT
    app.shares().share(alice, aliceDoc, bob, SharePermission::Edit);
    check(app.perms().canEditDocument(bob, aliceDoc), "Bob can edit EDIT-shared doc");

    // 11. SysAdmin đọc được mọi thứ
    check(app.perms().canReadDocument(admin, aliceDoc), "SysAdmin can read any doc");
    check(app.perms().canDeleteDocument(admin, pubDoc), "SysAdmin can delete any doc");

    // 12. Quota
    app.users().setQuotaLimit(alice, 100);
    check(app.quota().wouldExceed(alice, 1000000), "Quota would exceed after big upload");
    threw = false;
    try { app.docs().uploadPersonal(alice, "big", std::string(200, 'x')); }
    catch (...) { threw = true; }
    check(threw, "Upload blocked when quota exceeded");

    // 13. Search
    app.search().rebuildIndex();
    auto results = app.search().searchByContent(alice, "secret");
    check(!results.empty(), "Content search finds 'secret' in Alice Notes");

    std::cout << "\n=== " << passes << " passed, " << failures << " failed ===\n";
    return failures == 0 ? 0 : 1;
}
