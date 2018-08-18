add_subdirectory(menu)

target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/GuiInterface.cpp")