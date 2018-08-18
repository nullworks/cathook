target_sources(cathook PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/MicroPather/micropather.cpp")

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}/MicroPather")
target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}/ucccccp")
target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}/TF2_NavFile_Reader")

