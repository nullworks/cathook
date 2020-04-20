set(files "${CMAKE_CURRENT_LIST_DIR}/ConfigUI.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Manager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Registered.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Settings.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SettingCommands.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SettingsIO.cpp")
        
target_sources(cathook PRIVATE ${files})
list(REMOVE_ITEM ignore_files ${files})
set(ignore_files ${ignore_files} CACHE INTERNAL "")
