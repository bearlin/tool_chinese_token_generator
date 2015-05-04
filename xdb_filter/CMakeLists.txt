cmake_minimum_required(VERSION 2.8)

project(xdb_filter)

set(XDB_FILTER_ROOT
  ${PROJECT_SOURCE_DIR}
  CACHE STRING "XDB_FILTER_ROOT directory")

add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/libscws
  ${CMAKE_CURRENT_BINARY_DIR}/libscws)

set(XDB_FILTER_API_INCLUDES
  ${XDB_FILTER_ROOT}/
  CACHE STRING "XDB_FILTER_API_INCLUDES path")

include_directories(
  ${libscws_API_INCLUDES}
  ${XDB_FILTER_API_INCLUDES})

file(GLOB XDB_FILTER_HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB XDB_FILTER_SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME}
  ${XDB_FILTER_SOURCE_FILES}
  ${XDB_FILTER_HEADER_FILES})

target_link_libraries(${PROJECT_NAME}
  libscws)

