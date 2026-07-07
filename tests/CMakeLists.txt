cmake_minimum_required(VERSION 3.16)

# Smoke test: kiểm tra các kịch bản phân quyền chính.
# Link với thư viện dms_core để có sẵn toàn bộ service.
add_executable(dms_smoke_test smoke_test.cpp)
target_link_libraries(dms_smoke_test PRIVATE dms_core)

add_test(NAME dms_smoke_test COMMAND dms_smoke_test)
