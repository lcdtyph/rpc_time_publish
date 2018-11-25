cmake_minimum_required(VERSION 3.13.0)

project(rpc_time_publish CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(glog 0.3.4 REQUIRED)

add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(service)

