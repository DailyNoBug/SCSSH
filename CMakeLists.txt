cmake_minimum_required(VERSION 3.27)
project(SCSSH)

set(CMAKE_CXX_STANDARD 17)

set(LIBSSH_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libssh2)
message(STATUS "LIBSSH_TOP_DIR: ${LIBSSH_TOP_DIR}")
set(LIBSSH2_INCLUDE_DIR ${LIBSSH_TOP_DIR}/include
                        ${LIBSSH_TOP_DIR}/src)

add_subdirectory(thirdparty/libssh2)
include_directories(${LIBSSH2_INCLUDE_DIR})

add_executable(SCSSH main.cpp)
target_link_libraries(SCSSH libssh2::libssh2)

