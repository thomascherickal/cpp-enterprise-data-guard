cmake_minimum_required(VERSION 3.16)
project(cpp_enterprise_data_guard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build C++ unit tests" ON)
option(ENABLE_COVERAGE "Enable gcov code coverage instrumentation" OFF)
option(BUILD_ADMIN_GUI "Build GTK3 Admin Control Centre GUI" ON)

add_compile_options(-Wall -Wextra -Wpedantic)

if(ENABLE_COVERAGE AND NOT EMSCRIPTEN)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_library(dataguard_core STATIC
    src/core/json_value.cpp
    src/core/json_dump.cpp
    src/core/json_parser.cpp
    src/core/json_parser_tokens.cpp
    src/core/validators_checksum.cpp
    src/core/validators_entropy.cpp
    src/core/policy_io.cpp
    src/core/policy_validate.cpp
    src/core/rule_registry.cpp
    src/core/text_index.cpp
    src/core/redactor.cpp
    src/core/scanner_rules.cpp
    src/core/scanner_scoring.cpp
    src/core/scanner.cpp
)
target_include_directories(dataguard_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(dataguard_admin_lib STATIC
    src/admin_gui/admin_controller.cpp
    src/admin_gui/admin_cli.cpp
)
target_link_libraries(dataguard_admin_lib PUBLIC dataguard_core)
target_include_directories(dataguard_admin_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(NOT EMSCRIPTEN AND BUILD_ADMIN_GUI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    add_executable(dataguard-admin
        src/admin_gui/main.cpp
        src/admin_gui/actions.cpp
        src/admin_gui/ui_helpers.cpp
        src/admin_gui/ui_theme.cpp
        src/admin_gui/tab_policy.cpp
        src/admin_gui/tab_rules.cpp
        src/admin_gui/tab_test_bench.cpp
    )
    target_include_directories(dataguard-admin PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(dataguard-admin PRIVATE dataguard_admin_lib ${GTK3_LIBRARIES})
    target_compile_options(dataguard-admin PRIVATE ${GTK3_CFLAGS_OTHER})
endif()

if(BUILD_TESTING AND NOT EMSCRIPTEN)
    enable_testing()
    add_executable(dataguard_tests
        tests/cpp/test_main.cpp
        tests/cpp/test_json.cpp
        tests/cpp/test_json_errors.cpp
        tests/cpp/test_validators.cpp
        tests/cpp/test_policy.cpp
        tests/cpp/test_scanner.cpp
        tests/cpp/test_scanner_policy.cpp
        tests/cpp/test_admin_controller.cpp
        tests/cpp/test_admin_cli.cpp
    )
    target_link_libraries(dataguard_tests PRIVATE dataguard_admin_lib)
    add_test(NAME dataguard_unit_tests COMMAND dataguard_tests)
endif()
