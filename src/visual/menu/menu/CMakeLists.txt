file(GLOB_RECURSE Source ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

target_sources(cathook PRIVATE ${Source})