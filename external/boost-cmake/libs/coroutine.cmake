if(USE_WINDOWS)
  set(coroutine_srcs
    ${BOOST_SOURCE}/libs/coroutine/src/windows/stack_traits.cpp
  )
else()
  set(coroutine_srcs
    ${BOOST_SOURCE}/libs/coroutine/src/posix/stack_traits.cpp
  )
endif()

_add_boost_lib(
  NAME coroutine
  SOURCES
    ${coroutine_srcs}
    ${BOOST_SOURCE}/libs/coroutine/src/detail/coroutine_context.cpp
    ${BOOST_SOURCE}/libs/coroutine/src/exceptions.cpp
  DEFINE_PRIVATE
    BOOST_COROUTINES_SOURCE
  LINK
    Boost::context
    Boost::thread
)

_add_boost_test(
  NAME coroutine_test
  LINK
    Boost::coroutine
    Boost::unit_test_framework
  TESTS
    RUN ${BOOST_SOURCE}/libs/coroutine/test/test_asymmetric_coroutine.cpp
    RUN ${BOOST_SOURCE}/libs/coroutine/test/test_symmetric_coroutine.cpp
)
