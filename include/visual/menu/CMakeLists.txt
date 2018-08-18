target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(menu)

target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/GuiInterface.hpp")
