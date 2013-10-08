cmake_minimum_required (VERSION 2.8)
project (aminer-core)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS interface.proto)

include_directories("${SAE_SOURCE_DIR}/src/saedb")
include_directories("${PROTOBUF_INCLUDE_DIRS}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(servicebase "event.cpp" "event.hpp" ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(servicebase zrpc gflags google-glog ${PROTOBUF_LIBRARY})

file (GLOB aminer-core_SOURCES
    "main.cpp"
    "*.hpp"
    "*.cpp"
)

add_executable(aminer-core ${aminer-core_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(aminer-core zrpc google-glog gflags ${PROTOBUF_LIBRARY} mgraph indexing )
