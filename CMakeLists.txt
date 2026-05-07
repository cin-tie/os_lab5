cmake_minimum_required(VERSION 3.20)

project(os_lab5 LANGUAGES C CXX)

cmake_policy(SET CMP0135 NEW)

# -------------------------
# Language standards
# -------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# Windows / MinGW fixes
# -------------------------
if (MINGW)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    add_compile_definitions(_GNU_SOURCE)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# -------------------------
# Include directory (modern way)
# -------------------------
include_directories(${CMAKE_SOURCE_DIR}/include)

# -------------------------
# SERVER
# -------------------------
add_executable(server
    src/server.cpp
    src/file_manager.cpp
    src/lock_manager.cpp
    src/pipe_server.cpp
    src/utils.cpp
)

target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(server PRIVATE
    Threads::Threads
)

# -------------------------
# CLIENT
# -------------------------
add_executable(client
    src/client.cpp
    src/utils.cpp
)

target_include_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(client PRIVATE
    Threads::Threads
)

# Output directory
set_target_properties(server client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =========================================================
# TESTING
# =========================================================

include(FetchContent)

# GoogleTest config (IMPORTANT for MinGW stability)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests
    tests/unit_tests.cpp
    src/file_manager.cpp
    src/lock_manager.cpp
    src/utils.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(unit_tests PRIVATE
    gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

# -------------------------
# Custom test target
# -------------------------
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS unit_tests
)