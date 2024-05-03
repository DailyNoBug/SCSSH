cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 17)

set(TARGET_NAME SCSSH)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)
add_executable(${TARGET_NAME} ${SRC_LIST})

