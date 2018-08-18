target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/classinfo.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/dump.hpp")

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")