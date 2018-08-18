target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/colors.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/drawing.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/drawmgr.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/EffectChams.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/EffectGlow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/fidgetspinner.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/EventLogging.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/SDLHooks.cpp")

if(EnableGUI)
    add_subdirectory(menu)
endif()