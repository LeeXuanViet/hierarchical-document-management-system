cmake_minimum_required(VERSION 3.16)
project(HierarchicalDocManager CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Thư viện tĩnh chứa toàn bộ core sources, dùng chung cho app và test.
file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_SOURCE_DIR}/src/domain/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/repositories/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/services/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/api/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/storage/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/*.cpp"
)

add_library(dms_core STATIC ${CORE_SOURCES})
target_include_directories(dms_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(dms_core PUBLIC sqlite3)

# ─── Qt5 GUI ───────────────────────────────────────────────────────────────────
find_package(Qt5 COMPONENTS Widgets QUIET)

if(Qt5_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    file(GLOB GUI_SOURCES src/gui/*.cpp src/gui/*.hpp)

    add_executable(docmanager_gui ${GUI_SOURCES})
    target_link_libraries(docmanager_gui PRIVATE dms_core Qt5::Widgets)
    target_include_directories(docmanager_gui PRIVATE ${CMAKE_SOURCE_DIR}/src)

    message(STATUS "Qt5 found — building docmanager_gui")
else()
    message(STATUS "Qt5 NOT found — skipping GUI target (install qtbase5-dev)")
endif()

enable_testing()
add_subdirectory(tests)
