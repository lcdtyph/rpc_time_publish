cmake_minimum_required(VERSION 3.9)

project(rpc_time_publish CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(glog 0.3.4 REQUIRED)

set(gRPC_INSTALL OFF)
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(grpc)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP_UNSECURE grpc++_unsecure)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(service)

