cmake_minimum_required(VERSION 3.14)
project(GrepRust VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch cxxopts
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(cxxopts)

# Main executable
add_executable(grep
    src/main.cpp
    src/grep.cpp
)
target_link_libraries(grep PRIVATE cxxopts::cxxopts)

target_include_directories(grep
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Test executable
add_executable(grepTest
    tests/grepTests.cpp
    src/grep.cpp
)

target_include_directories(grepTest
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(grepTest
    PRIVATE
        GTest::gtest_main
)

# Add tests
include(GoogleTest)
gtest_discover_tests(grepTest)