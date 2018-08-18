file(GLOB_RECURSE Source ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

target_sources(cathook PRIVATE ${Source})