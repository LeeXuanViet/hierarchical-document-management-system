# Hierarchical Document Management System (C++17 + Qt5)

Hệ thống quản lý tài liệu phân cấp theo cây đơn vị tổ chức, tập trung vào mô hình phân quyền phức tạp (role-based + tree-based inheritance).

## Tính năng chính

- Quản lý cây đơn vị tổ chức (hierarchical OrgUnit)
- Phân quyền chi tiết: **SYS_ADMIN**, **UNIT_ADMIN**, **USER**
- Tài liệu **Personal** và **Unit Public** với quy tắc kế thừa quyền xem theo subtree
- Chia sẻ tài liệu với quyền **READ** hoặc **EDIT**
- Tìm kiếm theo tên và nội dung (hỗ trợ trích xuất text từ PDF)
- Quản lý quota lưu trữ theo người dùng
- Giao diện desktop Qt5: preview PDF, cây đơn vị, bảng tài liệu, admin panel
- Lưu trữ metadata bằng **SQLite3**, file thực tế bằng **FileStore**

## Yêu cầu hệ thống

- **C++17**
- **Qt5** (`qtbase5-dev`)
- **SQLite3** (`libsqlite3-dev`)
- **poppler-utils** (pdftotext, pdftoppm) để preview và trích xuất text PDF
- **CMake** 3.16+

## Build & Run

```bash
# 1. Cài dependencies (Ubuntu/Debian)
sudo apt update
sudo apt install build-essential cmake qtbase5-dev libsqlite3-dev poppler-utils

# 2. Build
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# 3. Chạy GUI
./docmanager_gui
```

Lần đầu chạy sẽ tự động seed dữ liệu mẫu (cây đơn vị, người dùng, tài liệu demo).

## Tài khoản demo

| Vai trò    | Username | Password | Đơn vị       |
|------------|----------|----------|--------------|
| SYS_ADMIN  | admin    | admin123 | Headquarters |
| UNIT_ADMIN | leadA    | lead123  | DivisionA    |
| USER       | alice    | alice123 | TeamA1       |
| USER       | bob      | bob123   | TeamA2       |
| USER       | carol    | carol123 | DivisionB    |

## Cấu trúc dự án

```
src/
├── domain/        # Entity (User, OrgUnit, Document, ShareRecord, FavoriteRecord) và Enum
├── repositories/  # Repository Pattern
│   ├── interfaces/ # Các interface (IUserRepository, IDocumentRepository, ...)
│   └── sqlite/     # Triển khai SQLite
├── services/      # Logic nghiệp vụ (PermissionService là cốt lõi)
├── gui/           # Giao diện Qt5 (MainWindow, Dialogs, Widgets, PDF renderer)
├── api/           # AppContext (DI container) + seed_data
├── storage/       # FileStore quản lý file thực tế
└── utils/         # IdGenerator, Hash, Tokenizer
```

## Kiến trúc

Hệ thống theo mô hình phân tầng **GUI → Service → Repository → Domain**, kết nối qua `AppContext` (Dependency Injection container).

- **PermissionService** — engine phân quyền trung tâm, kiểm tra theo thứ tự: SYS_ADMIN → OWNER → SHARED → UNIT_PUBLIC (theo quan hệ ancestor-descendant trên cây đơn vị).
- **SessionService** — xác thực và quản lý phiên đăng nhập.
- **SQLite3** — WAL mode, bật foreign keys.

## Công nghệ

- C++17 + CMake
- Qt5 Widgets
- SQLite3
- poppler-utils (PDF)
- Repository Pattern + Layered Architecture
