set(files "${CMAKE_CURRENT_LIST_DIR}/C_MannVsMachineStats.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/CTFInventoryManager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/CTFPartyClient.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/CTFParty.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/CTFGCClientSystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ITFGroupMatchCriteria.cpp")
target_sources(cathook PRIVATE ${files})
