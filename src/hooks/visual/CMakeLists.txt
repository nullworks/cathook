target_sources(cathook PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/BeginFrame.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/DrawModelExecute.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/FrameStageNotify.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/OverrideView.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/PaintTraverse.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/RandomInt.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/SDL_GL_SwapWindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/SDL_PollEvent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/IN_KeyEvent.cpp")