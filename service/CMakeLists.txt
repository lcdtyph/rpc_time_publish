cmake_minimum_required(VERSION 3.9)

project(tpservice)

list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/tpservice.cc)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} time_publish_proto_objs glog::glog)

