cmake_minimum_required(VERSION 2.8)
project(aminer-core)

add_executable(aminer_importer "aminer_importer.cpp" "../../service/aminer.hpp" "csv_reader.hpp")
target_link_libraries(aminer_importer mgraph)

