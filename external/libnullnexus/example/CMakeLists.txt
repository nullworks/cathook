cmake_minimum_required(VERSION 3.10)
project (client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/libnullnexus)

target_link_libraries(client PRIVATE libnullnexus)
