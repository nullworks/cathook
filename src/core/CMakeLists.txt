set(files "${CMAKE_CURRENT_LIST_DIR}/cvwrapper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/entry.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/init.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/interfaces.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/logging.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/netvars.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/overwrite_dlopen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/profiler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/sharedobj.cpp")
target_sources(cathook PRIVATE ${files})
