target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/atlas.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/colors.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/drawing.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/drawmgr.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/EffectChams.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/EffectGlow.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/fidgetspinner.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/EventLogging.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/SDLHooks.hpp")

if(EnableGUI)
    add_subdirectory(menu)
endif()

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
