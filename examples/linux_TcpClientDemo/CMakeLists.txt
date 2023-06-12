# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.8)

# 项目信息
project(tp_pipeline_demo)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_FLAGS "-Wall -fexceptions -fPIC -std=c++11")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# add_definitions(-m64)

LINK_DIRECTORIES( /usr/lib )
INCLUDE_DIRECTORIES( /usr/include )

LINK_DIRECTORIES( /usr/local/lib )
INCLUDE_DIRECTORIES( /usr/local/include )

# 根目录/代码路径
aux_source_directory(. MAIN_SRCS)

# 添加子目录
INCLUDE_DIRECTORIES(./)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../src)

# 添加子代码路径
aux_source_directory(../../src/util/tool_string DIR_SRCS)
aux_source_directory(../../src/util/tool_net DIR_SRCS)

aux_source_directory(../../src/tcp/linux_socket_base DIR_SRCS)
aux_source_directory(../../src/tcp/linux_epoll_client DIR_SRCS)
aux_source_directory(../../src/tcp/socket_conn_manage_base DIR_SRCS)

aux_source_directory(../../src/threadpool/thread_pool_base DIR_SRCS)

aux_source_directory(../../src/tcp/simple_net_pack DIR_SRCS)

set(OTHER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/util/tool_string/sof_string.cpp
)

SET(EXECUTABLE_OUTPUT_PATH ../)

#
LINK_LIBRARIES(
    pthread
)

add_executable(tppl_demo ${MAIN_SRCS} ${DIR_SRCS} ${OTHER_SRCS})
