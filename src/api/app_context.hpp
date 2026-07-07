#pragma once

#include "../repositories/sqlite/sqlite_database.hpp"
#include "../repositories/sqlite/sqlite_org_unit_repository.hpp"
#include "../repositories/sqlite/sqlite_user_repository.hpp"
#include "../repositories/sqlite/sqlite_document_repository.hpp"
#include "../repositories/sqlite/sqlite_share_repository.hpp"
#include "../repositories/sqlite/sqlite_favorite_repository.hpp"
#include "../storage/file_store.hpp"
#include "../utils/id_generator.hpp"
#include "../services/org_tree_service.hpp"
#include "../services/user_service.hpp"
#include "../services/permission_service.hpp"
#include "../services/document_service.hpp"
#include "../services/share_service.hpp"
#include "../services/search_service.hpp"
#include "../services/session_service.hpp"
#include "../services/quota_service.hpp"

#include <string>
#include <memory>

namespace dms {

// Container tập trung: sở hữu toàn bộ repository, service và wiring.
// Dữ liệu được lưu trữ trong SQLite database (persistent).
class AppContext {
public:
    explicit AppContext(std::string dbPath = "dms.db", std::string fileDir = "data/files")
        : db_(std::move(dbPath)),
          fileStore_(std::move(fileDir)),
          orgRepo_(db_), userRepo_(db_), docRepo_(db_), shareRepo_(db_), favRepo_(db_),
          idGenUnit_("unit"), idGenUser_("user"), idGenDoc_("doc"), idGenShare_("share"),
          orgTree_(orgRepo_, idGenUnit_),
          users_(userRepo_, orgRepo_, idGenUser_),
          perms_(userRepo_, docRepo_, shareRepo_, orgTree_),
          docs_(docRepo_, userRepo_, favRepo_, fileStore_, idGenDoc_, perms_, users_),
          shares_(shareRepo_, docRepo_, userRepo_, idGenShare_, perms_),
          search_(docRepo_, fileStore_, perms_),
          session_(userRepo_),
          quota_(users_) {}

    // Returns true if the database was freshly created (no existing data).
    bool isNewDatabase() const { return db_.isNewDatabase(); }

    // Repositories
    IOrgUnitRepository& orgRepo() { return orgRepo_; }
    IUserRepository& userRepo() { return userRepo_; }
    IDocumentRepository& docRepo() { return docRepo_; }
    IShareRepository& shareRepo() { return shareRepo_; }
    IFavoriteRepository& favRepo() { return favRepo_; }

    // Services
    OrgTreeService& orgTree() { return orgTree_; }
    UserService& users() { return users_; }
    PermissionService& perms() { return perms_; }
    DocumentService& docs() { return docs_; }
    ShareService& shares() { return shares_; }
    SearchService& search() { return search_; }
    SessionService& session() { return session_; }
    QuotaService& quota() { return quota_; }
    FileStore& fileStore() { return fileStore_; }

private:
    SqliteDatabase db_;
    FileStore fileStore_;

    SqliteOrgUnitRepository orgRepo_;
    SqliteUserRepository userRepo_;
    SqliteDocumentRepository docRepo_;
    SqliteShareRepository shareRepo_;
    SqliteFavoriteRepository favRepo_;

    IdGenerator idGenUnit_;
    IdGenerator idGenUser_;
    IdGenerator idGenDoc_;
    IdGenerator idGenShare_;

    OrgTreeService orgTree_;
    UserService users_;
    PermissionService perms_;
    DocumentService docs_;
    ShareService shares_;
    SearchService search_;
    SessionService session_;
    QuotaService quota_;
};

} // namespace dms
