cmake_minimum_required(VERSION 2.8)

project(xdb_dump)

set(XDB_DUMP_ROOT
  ${PROJECT_SOURCE_DIR}
  CACHE STRING "XDB_DUMP_ROOT directory")

set(XDB_DUMP_API_INCLUDES
  ${XDB_DUMP_ROOT}/
  CACHE STRING "XDB_DUMP_API_INCLUDES path")

include_directories(
  ${XDB_DUMP_API_INCLUDES})

file(GLOB XDB_DUMP_HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB XDB_DUMP_SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME}
  ${XDB_DUMP_SOURCE_FILES}
  ${XDB_DUMP_HEADER_FILES})

#target_link_libraries(${PROJECT_NAME}
#  libscws)

