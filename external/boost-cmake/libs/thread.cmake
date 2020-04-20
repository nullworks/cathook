if(USE_WINDOWS)
  set(THREAD_SRCS
    ${BOOST_SOURCE}/libs/thread/src/win32/thread.cpp
    ${BOOST_SOURCE}/libs/thread/src/win32/tss_pe.cpp
  )
else()
  set(THREAD_SRCS
    ${BOOST_SOURCE}/libs/thread/src/pthread/thread.cpp
    ${BOOST_SOURCE}/libs/thread/src/pthread/once.cpp
  )
endif()

_add_boost_lib(
  NAME thread
  SOURCES
    ${THREAD_SRCS}
    ${BOOST_SOURCE}/libs/thread/src/future.cpp
  DEFINE_PRIVATE
    BOOST_THREAD_BUILD_LIB=1
  LINK
    Boost::chrono
)
if(NOT USE_WINDOWS)
  find_package(Threads REQUIRED)

  target_link_libraries(Boost_thread PUBLIC Threads::Threads)
  target_compile_definitions(Boost_thread PRIVATE
    BOOST_THREAD_POSIX
  )
endif()
