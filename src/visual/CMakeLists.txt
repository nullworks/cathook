set(files "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/colors.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/drawing.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/drawmgr.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/EffectGlow.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/EventLogging.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/picopng.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SDLHooks.cpp")

if(EnableGUI)
    add_subdirectory(menu)
    list(APPEND files "${CMAKE_CURRENT_LIST_DIR}/GUISnow.cpp")
endif()

target_sources(cathook PRIVATE ${files})

if (EnableImGuiDrawing)
    add_subdirectory(imgui)
endif()
