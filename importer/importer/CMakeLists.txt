cmake_minimum_required(VERSION 2.8)
project(aminer-core)


find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS interface.proto)

#add_executable(aminerserver "aminer.hpp" "aminerdata.hpp" "aminerservice.cpp")
#target_link_libraries(aminerserver servicebase mgraph indexing)

add_executable(aminer_importer "aminer_importer.cpp" "../../service/aminer.hpp" "csv_reader.hpp")
target_link_libraries(aminer_importer mgraph)

