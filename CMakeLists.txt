message(STATUS "Building using CMake version: ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

project(nanoarrow_perf_benchmark)

fetchcontent_declare(
        nanoarrow
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/arrow-nanoarrow)
fetchcontent_makeavailable(nanoarrow)
fetchcontent_declare(
        nanoarrow_ipc
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/arrow-nanoarrow/extensions/nanoarrow_ipc)
fetchcontent_makeavailable(nanoarrow_ipc)

include_directories(src)

add_executable(nanoarrow_perf_benchmark
        src/main.cpp src/converters.hpp)

target_link_libraries(
        nanoarrow_perf_benchmark PRIVATE nanoarrow nanoarrow_ipc
)
