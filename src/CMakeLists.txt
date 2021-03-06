cmake_minimum_required(VERSION 2.8)
project(aminer-core)

add_subdirectory (importer)

# boost
#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package( Boost 1.46 COMPONENTS regex thread system REQUIRED)
#include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS interface.proto)

include_directories("${SAE_SOURCE_DIR}/src/saedb")
include_directories("${PROTOBUF_INCLUDE_DIRS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
#include_directories("${aminer-core_SOURCE_DIR}/src/db")
include_directories("${aminer-core_SOURCE_DIR}/src/search")
include_directories("${aminer-core_SOURCE_DIR}/src/service")
include_directories("${aminer-core_SOURCE_DIR}/src/importer")
#include_directories("${aminer-core_SOURCE_DIR}/src/util")

file (GLOB aminer-core_SOURCES
    "main.cpp"
    "search/*.h"
    "search/*.cpp"
    "service/*.h"
    "service/*.cpp"
    "importer/aminer_data.hpp"
    "importer/aminer.hpp"
)

add_executable(aminer-core ${aminer-core_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(aminer-core zrpc glog gflags ${PROTOBUF_LIBRARY} mgraph indexing )

