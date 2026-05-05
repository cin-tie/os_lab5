cmake_minimum_required(VERSION 3.9)

project(os_lab5)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_COMPILER
/usr/bin/x86_64-w64-mingw32-gcc)

set(CMAKE_CXX_COMPILER
/usr/bin/x86_64-w64-mingw32-g++)

set(CMAKE_CXX_FLAGS
"${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

set(CMAKE_EXE_LINKER_FLAGS
"${CMAKE_EXE_LINKER_FLAGS} -static")

include_directories(
${CMAKE_SOURCE_DIR}/include
)

add_executable(server
src/server.cpp
src/file_manager.cpp
src/lock_manager.cpp
src/pipe_server.cpp
src/utils.cpp
)

add_executable(client
src/client.cpp
src/utils.cpp
)

set_target_properties(
server client
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY
${CMAKE_BINARY_DIR}/bin
)

# GoogleTest setup
include(FetchContent)

FetchContent_Declare(
googletest
URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Unit tests executable (без копирования файлов)
add_executable(unit_tests
tests/unit_tests.cpp
src/file_manager.cpp
src/lock_manager.cpp
src/utils.cpp
)

target_link_libraries(unit_tests
gtest
gtest_main
)

add_test(NAME unit_tests COMMAND unit_tests)

# Optional: Run tests with verbose output
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS unit_tests
)

# RUN 
# cmake .. -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++