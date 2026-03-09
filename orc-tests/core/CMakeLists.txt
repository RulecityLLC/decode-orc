cmake_minimum_required(VERSION 3.20)

project(orc-core-tests)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# See https://google.github.io/googletest/quickstart-cmake.html

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        orc-core-tests

        stages/daphne_vbi_sink/daphne_vbi_writer_util_test.cpp
)

target_link_libraries(
        orc-core-tests
        orc-core
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(orc-core-tests)