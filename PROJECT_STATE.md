# PROJECT STATE - Hierarchical Document Management System

## Status: 100% COMPLETE & WORKING
- Build: OK (0 errors, 0 warnings)
- Tests: 22/22 PASS
- GUI: Qt5 desktop application
- Ready for defense presentation

## Context
- Viettel Digital Talent Track - System Programming (C++)
- Project: Hierarchical Document Management System
- Tree-structured organizational units with role-based access control
- SQLite persistence + Qt5 GUI desktop application

## Build & Run Instructions
```bash
cd /home/viet/vdt_cpp/cpp_test
mkdir -p build && cd build
cmake .. && make -j$(nproc)
./docmanager_gui          # Qt5 GUI desktop application
cd tests && ctest         # Run 22 smoke tests
```

## Deterministic IDs After Seed
```
Units:  unit_1=HQ, unit_2=DivisionA, unit_3=DivisionB, unit_4=TeamA1, unit_5=TeamA2
Users:  user_1=admin(SysAdmin,HQ), user_2=leadA(UnitAdmin,DivisionA),
        user_3=alice(User,TeamA1), user_4=bob(User,TeamA2), user_5=carol(User,DivisionB)
Docs:   doc_1=DivisionA Policy(UnitPublic), doc_2=Alice Notes(Personal)
Login:  admin/admin123, leadA/lead123, alice/alice123, bob/bob123, carol/carol123
```

## Architecture
```
src/
├── domain/          # Entities & enums (OrgUnit, User, Document, ShareRecord, FavoriteRecord)
├── repositories/
│   ├── interfaces/  # Abstract repository interfaces (IOrgUnit, IUser, IDocument, IShare, IFavorite)
│   └── sqlite/      # SQLite implementations (WAL mode, foreign keys, prepared statements)
├── utils/           # IdGenerator(deterministic prefix_counter), Hash(FNV-1a), Tokenizer
├── storage/         # FileStore (POSIX file I/O for document content)
├── services/        # Business logic (OrgTree, User, Permission, Document, Share, Search, Session, Quota)
├── api/             # AppContext (DI container) + seed_data (shared demo data seeding)
└── gui/             # Qt5 Widgets GUI (MainWindow, LoginDialog, AdminPanel, DocumentTable, SearchPanel, etc.)
```

## Key Design Decisions
1. **Deterministic IDs**: prefix_counter format (unit_1, user_2, doc_3) for reproducible demos
2. **Permission Engine**: Central PermissionService with explainReadAccess returning OWNER/SHARED/UNIT_PUBLIC/SYS_ADMIN/DENIED
3. **Visibility Inheritance**: UnitPublic docs of unit U visible to all descendant units (isAncestorOrSelf check)
4. **Inverted Index**: SearchService tokenizes content, AND-logic for multi-word queries
5. **Quota Enforcement**: Per-user byte limit, checked on upload/edit
6. **SQLite Persistence**: All metadata stored in SQLite (WAL mode, foreign keys, prepared statements); file content in `data/files` via FileStore; search index rebuilt on startup
7. **Qt5 GUI**: Desktop application with login, org tree navigation, document table, search panel, admin panel, PDF preview (via poppler-utils)
8. **Shared Seed Data**: `src/api/seed_data.hpp/cpp` provides unified `dms::seedDemo()` used by GUI; seeds only when database is freshly created

## File Listing with Descriptions

### CMakeLists.txt
- C++17, static library `dms_core` containing all core sources (domain, repositories, services, api, storage, utils)
- `docmanager_gui` executable links against dms_core + Qt5::Widgets
- tests subdirectory also links against dms_core
- Links sqlite3 for database persistence

### src/domain/
- `enums.hpp/.cpp` - Role(User/UnitAdmin/SysAdmin), VisibilityType(Personal/UnitPublic/DirectShare), SharePermission(Read/Edit) with string conversions
- `org_unit.hpp` - struct {id, name, parentUnitId, childrenIds}
- `user.hpp` - struct {id, username, passwordHash, role, unitId, quotaLimit, quotaUsed}
- `document.hpp` - struct {id, ownerUserId, unitScopeId, visibility, title, contentPath, contentType, size, createdAt, updatedAt, isDeleted}
- `share_record.hpp` - struct {id, documentId, fromUserId, toUserId, permission}
- `favorite_record.hpp` - struct {userId, documentId}

### src/repositories/interfaces/
- `i_org_unit_repository.hpp` - save/findById/findAll/remove
- `i_user_repository.hpp` - save/findById/findByUsername/findAll/findByUnit/remove
- `i_document_repository.hpp` - save/findById/findAll/findByOwner/findByUnitScope/remove
- `i_share_repository.hpp` - save/findByDocument/findByToUser/findByFromUser/remove
- `i_favorite_repository.hpp` - add/remove/exists/findByUser

### src/repositories/sqlite/
- `sqlite_database.hpp/.cpp` - SQLite connection management, schema creation (WAL mode, foreign keys)
- `sqlite_user_repository.hpp/.cpp` - User CRUD via prepared statements
- `sqlite_org_unit_repository.hpp/.cpp` - OrgUnit CRUD with children population, ORDER BY name
- `sqlite_document_repository.hpp/.cpp` - Document CRUD with unit scope queries
- `sqlite_share_repository.hpp/.cpp` - Share record CRUD
- `sqlite_favorite_repository.hpp/.cpp` - Favorite record CRUD

### src/utils/
- `id_generator.hpp/.cpp` - IdGenerator(prefix): next() returns "prefix_N" (N increments from 1)
- `hash.hpp/.cpp` - FNV-1a 64-bit hash, simpleHash() + verifyPassword()
- `tokenizer.hpp/.cpp` - tokenize(text)->vector<string>, toLower, trim, containsIgnoreCase

### src/storage/
- `file_store.hpp/.cpp` - FileStore(baseDir): write(docId,content)->path, read(path)->content, remove(path), size(path)
- Creates directories with POSIX mkdir, stores as docId.txt

### src/services/
- `org_tree_service.hpp/.cpp` - createUnit, getTree(DFS), getSubtree, getAncestors, isAncestorOrSelf, exists, getUnit
- `user_service.hpp/.cpp` - createUser(hash password), changeRole, changeUnit, setQuotaLimit, adjustQuotaUsed, getUser, findByUsername, findAll, findByUnit, remove
- `permission_service.hpp/.cpp` - canRead/Edit/DeleteDocument, canUploadUnitPublic, isSysAdmin, isUnitAdminOf, explainReadAccess
- `document_service.hpp/.cpp` - uploadPersonal, uploadUnitPublic, edit, remove, readContent, getDocument, listMine, listVisibleUnitPublic, addFavorite, removeFavorite, listFavorites, preview
- `share_service.hpp/.cpp` - share(dedup existing), revoke, listSharedWithMe, listSharedByMe
- `search_service.hpp/.cpp` - rebuildIndex, indexDocument, removeFromIndex, searchByTitle(substring), searchByContent(inverted index AND), setTextExtractor (PDF text extraction callback)
- `session_service.hpp/.cpp` - login(verify hash), logout, currentUserId, isLoggedIn
- `quota_service.hpp/.cpp` - used, limit, usagePercent, wouldExceed, formatBytes(static)

### src/api/
- `app_context.hpp` - Central DI container owning all repos+services, wired in constructor initializer list; takes dbPath and fileDir parameters
- `seed_data.hpp/.cpp` - Shared `dms::seedDemo(AppContext&)` function: creates org tree (HQ→DivisionA→{TeamA1,TeamA2}, DivisionB), 5 users, 2 documents, rebuilds search index

### src/gui/
- `main_gui.cpp` - GUI entry point: initializes AppContext with SQLite, registers PDF text extractor, seeds demo data if new DB, shows LoginDialog then MainWindow
- `login_dialog.hpp/.cpp` - Login dialog with username/password, validates against SessionService
- `main_window.hpp/.cpp` - Main window with toolbar tabs (Documents/Search/Admin), status bar showing user info and quota
- `org_tree_widget.hpp/.cpp` - QTreeWidget displaying org unit hierarchy
- `document_table_widget.hpp/.cpp` - QTableWidget showing documents for selected unit, upload/delete/favorite/share actions
- `document_detail_dialog.hpp/.cpp` - Document detail dialog with metadata, PDF preview (all pages), text content display
- `search_panel.hpp/.cpp` - Search panel with title/content search, results table
- `admin_panel.hpp/.cpp` - Admin panel for managing org units and users (create/delete units, create/delete users with quota)
- `share_dialog.hpp/.cpp` - Share dialog for granting READ/EDIT access to other users
- `pdf_renderer.hpp/.cpp` - PDF rendering via poppler-utils (pdftoppm for images, pdftotext for text extraction)
- `pdf_text_extractor.hpp/.cpp` - PDF text extraction utilities

### tests/
- `CMakeLists.txt` - links smoke_test against dms_core
- `smoke_test.cpp` - 22 assertions: ancestor checks, visibility inheritance, permission deny, share READ/EDIT, SysAdmin omniaccess, quota enforcement, content search

## Permission Rules Summary
1. **SysAdmin**: full access to everything (including Personal documents)
2. **Owner**: full access to own documents
3. **UnitAdmin**: can upload/edit/delete UnitPublic docs of their own unit
4. **UnitPublic visibility**: user in unit U sees public docs of U and all ancestor units (isAncestorOrSelf(docUnit, userUnit))
5. **DirectShare**: explicit share grants READ or EDIT to specific user
6. **Personal docs**: only visible to owner (and SysAdmin)

## Potential Next Steps
- [ ] Audit log (track all permission-changing actions)
- [ ] Document versioning
- [ ] Document classification/tagging
- [ ] Unit tests with Google Test
- [ ] CI/CD pipeline
