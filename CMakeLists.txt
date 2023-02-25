cmake_minimum_required(VERSION 3.2)

project(gears)

set(CMAKE_CXX_STANDARD 17)

find_package(plog CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

set(GEARS_SRCS
    src/http_client.cpp
    src/rpc_client.cpp
    src/utils.cpp
)

add_library(gears ${GEARS_SRCS})
target_link_libraries(gears PRIVATE plog::plog JsonCpp::JsonCpp CURL::libcurl)
