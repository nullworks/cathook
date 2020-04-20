_add_boost_lib(
  NAME exception
  SOURCES
    ${BOOST_SOURCE}/libs/exception/src/clone_current_exception_non_intrusive.cpp
)

_add_boost_test(
  NAME exception_test
  LINK
    Boost::exception
    Boost::thread
  TESTS
    RUN ${BOOST_SOURCE}/libs/exception/test/is_output_streamable_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/has_to_string_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/to_string_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/to_string_stub_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/1-throw_exception_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/2-throw_exception_no_exceptions_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/3-throw_exception_no_integration_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/4-throw_exception_no_both_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/cloning_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/copy_exception_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/unknown_exception_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/exception_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/enable_error_info_test.cpp
        ${BOOST_SOURCE}/libs/exception/test/helper1.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/throw_exception_test.cpp
        ${BOOST_SOURCE}/libs/exception/test/helper2.cpp
    # Fails to build on Android x86
    # RUN ${BOOST_SOURCE}/libs/exception/test/errno_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/error_info_lv_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/error_info_lv_const_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/error_info_rv_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/error_info_rv_const_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/diagnostic_information_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/refcount_ptr_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/current_exception_cast_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/errinfos_test.cpp
    RUN ${BOOST_SOURCE}/libs/exception/test/exception_ptr_test.cpp
)
