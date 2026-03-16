cmake_minimum_required(VERSION 3.15)
project(influx_cpp_writer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(influx_cpp_writer src/main.cpp)

target_link_libraries(influx_cpp_writer PRIVATE CURL::libcurl)

target_compile_features(influx_cpp_writer PRIVATE cxx_std_17)

install(TARGETS influx_cpp_writer RUNTIME DESTINATION bin)
