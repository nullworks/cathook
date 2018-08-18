target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/Bool.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Float.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Int.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Key.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Manager.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Registered.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Settings.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/SettingsIO.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/String.hpp")

if (EnableVisuals)
    target_sources(cathook PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/Rgba.hpp")
endif()

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")