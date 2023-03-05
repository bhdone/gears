cmake_minimum_required(VERSION 3.2)

project(gears)

option(ENABLE_TEST "Compile and run tests" ON)

set(CMAKE_CXX_STANDARD 17)

include(ExternalProject)
ExternalProject_Add(
    univalue
    GIT_REPOSITORY https://github.com/jgarzik/univalue
    GIT_TAG d6715ee16ed57bfbfc64e023dcb05151b6654aa4
    CONFIGURE_COMMAND test ! -f configure && ./autogen.sh && ./configure || exit 0
    BUILD_IN_SOURCE 1
)

ExternalProject_Get_Property(univalue SOURCE_DIR)
set(UniValue_Include ${SOURCE_DIR}/include)

ExternalProject_Add(
    tinyformat
    GIT_REPOSITORY https://github.com/c42f/tinyformat
    GIT_TAG aef402d85c1e8f9bf491b72570bfe8938ae26727
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)
ExternalProject_Get_Property(tinyformat SOURCE_DIR)
set(tinyformat_Include ${SOURCE_DIR})

include_directories(${UniValue_Include} ${tinyformat_Include})

set(GEARS_SRCS
    src/http_client.cpp
    src/rpc_client.cpp
    src/utils.cpp
)

add_library(gears ${GEARS_SRCS})
add_dependencies(gears univalue tinyformat)

if (ENABLE_TEST)
    enable_testing()

    find_package(GTest CONFIG REQUIRED)
    find_package(CURL CONFIG REQUIRED)
    find_library(UniValue_Lib NAMES univalue HINTS ${SOURCE_DIR})

    set(GEARS_TEST_SRCS
        ${GEARS_SRCS}
        ./src/gears_test.cpp
    )

    add_executable(gears_test ${GEARS_TEST_SRCS})
    target_link_libraries(gears_test PRIVATE GTest::gtest GTest::gtest_main CURL::libcurl ${UniValue_Lib})
endif()
