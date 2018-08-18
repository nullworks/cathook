target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/aimbot.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/antiaim.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/bhop.hpp")

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")