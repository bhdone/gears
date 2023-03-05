cmake_minimum_required(VERSION 3.2)

project(gears)

option(ENABLE_TEST "Compile and run tests" ON)

set(CMAKE_CXX_STANDARD 17)

find_package(plog CONFIG REQUIRED)

set(GEARS_SRCS
    src/http_client.cpp
    src/rpc_client.cpp
    src/utils.cpp
)

add_library(gears ${GEARS_SRCS})
target_link_libraries(gears PRIVATE plog::plog)

if (ENABLE_TEST)
    enable_testing()

    find_package(GTest CONFIG REQUIRED)
    find_package(jsoncpp CONFIG REQUIRED)
    find_package(CURL CONFIG REQUIRED)

    set(GEARS_TEST_SRCS
        ./src/utils.cpp
        ./src/http_client.cpp
        ./src/rpc_client.cpp
        ./src/gears_test.cpp
    )

    add_executable(gears_test ${GEARS_TEST_SRCS})
    target_link_libraries(gears_test PRIVATE GTest::gtest GTest::gtest_main plog::plog JsonCpp::JsonCpp CURL::libcurl)
endif()
