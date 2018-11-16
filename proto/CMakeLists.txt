cmake_minimum_required(VERSION 3.9)

project(time_publish_proto)

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
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}"
         "--cpp_out=${PROTO_OUTPUT_PATH}"
         "${PROTO_SRC}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}"
         "--grpc_out=${PROTO_OUTPUT_PATH}"
         "--plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${PROTO_SRC}"
    )

list(APPEND DEPS ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})
include_directories(${PROTO_OUTPUT_PATH})

add_library(${PROJECT_NAME} STATIC ${PROTO_OUTPUT_SOURCE})
target_link_libraries(${PROJECT_NAME} ${DEPS})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROTO_OUTPUT_PATH})

