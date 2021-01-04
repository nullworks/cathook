set(files "${CMAKE_CURRENT_LIST_DIR}/checksum_crc.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/checksum_md5.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/convar.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/HUD.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/KeyValues.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MaterialSystemUtil.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/tier1.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/utlbuffer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/UtlString.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/netmessage.cpp")
target_sources(cathook PRIVATE ${files})
