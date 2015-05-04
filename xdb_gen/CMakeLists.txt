cmake_minimum_required(VERSION 2.8)

project(xdb_gen)

set(XDB_GEN_ROOT
  ${PROJECT_SOURCE_DIR}
  CACHE STRING "XDB_GEN_ROOT directory")

set(XDB_GEN_API_INCLUDES
  ${XDB_GEN_ROOT}/
  CACHE STRING "XDB_GEN_API_INCLUDES path")

include_directories(
  ${XDB_GEN_API_INCLUDES})

file(GLOB XDB_GEN_HEADER_FILES ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB XDB_GEN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME}
  ${XDB_GEN_SOURCE_FILES}
  ${XDB_GEN_HEADER_FILES})

#target_link_libraries(${PROJECT_NAME}
#  libscws)

