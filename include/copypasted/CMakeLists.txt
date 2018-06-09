target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/CSignature.h"
        "${CMAKE_CURRENT_LIST_DIR}/CDumper.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/Netvar.h"
        "${CMAKE_CURRENT_LIST_DIR}/xorstring.h")

target_include_directories(cathook PRIVATE "${CMAKE_CURRENT_LIST_DIR}")