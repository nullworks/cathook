set(files "${CMAKE_CURRENT_LIST_DIR}/Manager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Registered.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Settings.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SettingCommands.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SettingsIO.cpp")
        
target_sources(cathook PRIVATE ${files})

if(EnableVisuals)
    set(files "${CMAKE_CURRENT_LIST_DIR}/ConfigUI.cpp")
    target_sources(cathook PRIVATE ${files})
endif()
