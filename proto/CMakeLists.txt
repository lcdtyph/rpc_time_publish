cmake_minimum_required(VERSION 3.13.0)

project(time_publish_proto)

find_package(Protobuf REQUIRED)

find_file(GRPC_PLUGIN_PATH grpc_cpp_plugin)

set(PROTO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/time_publish.proto")
set(PROTO_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/gen")
file(MAKE_DIRECTORY "${PROTO_OUTPUT_PATH}")

foreach(mid "pb" "grpc.pb")
    list(APPEND PROTO_OUTPUT_HEADER "${PROTO_OUTPUT_PATH}/time_publish.${mid}.h")
    list(APPEND PROTO_OUTPUT_SOURCE "${PROTO_OUTPUT_PATH}/time_publish.${mid}.cc")
endforeach()

add_custom_command(
    OUTPUT  ${PROTO_OUTPUT_HEADER}
            ${PROTO_OUTPUT_SOURCE}
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}"
         "--cpp_out=${PROTO_OUTPUT_PATH}"
         "${PROTO_SRC}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}"
         "--grpc_out=${PROTO_OUTPUT_PATH}"
         "--plugin=protoc-gen-grpc=${GRPC_PLUGIN_PATH}"
         "${PROTO_SRC}"
    )

list(APPEND DEPS grpc++_unsecure grpc_unsecure ${PROTOBUF_LIBRARY})
include_directories(${PROTO_OUTPUT_PATH})

add_library(${PROJECT_NAME}_objs OBJECT ${PROTO_OUTPUT_SOURCE})
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${DEPS})
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${PROTO_OUTPUT_PATH})

