cmake_minimum_required (VERSION 2.8)

project (aminer-core)

# binary output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${aminer-core_BINARY_DIR}")

# try enable c++11
# g++ and clang++ need flags to enable c++11
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" HAS_CPP11)
if (HAS_CPP11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# use libc++ on Mac OS X
# by default clang++ will use libstdc++ w/o c++11 support
if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

# find saedb library
set(SAEDB_ROOT_PATH "../saedb" CACHE FILEPATH "root of saedb library")

add_subdirectory (${SAEDB_ROOT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/saedb)
# add includes for google-glog
include_directories ("${google-glog_SOURCE_DIR}/src")
# add includes for gflags
include_directories ("${gflags_BINARY_DIR}/include")
# add includes for sae
include_directories ("${SAEDB_ROOT_PATH}")

set(CMAKE_LIBRARY_PATH "${gflags_BINARY_DIR}/lib" "${google-glog_BINARY_DIR}" ${CMAKE_LIBRARY_PATH})

# add the core libraries
add_subdirectory (importer/importer)
add_subdirectory (service)
