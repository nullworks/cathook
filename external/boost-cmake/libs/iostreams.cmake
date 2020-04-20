option(BOOST_IOSTREAMS_ENABLE_BZIP2 "Enable bzip2 support in Boost Iostreams if available" ON)
option(BOOST_IOSTREAMS_ENABLE_ZLIB "Enable zlib support in Boost Iostreams if available" ON)

_add_boost_lib(
  NAME iostreams
  SOURCES
    ${BOOST_SOURCE}/libs/iostreams/src/file_descriptor.cpp
    ${BOOST_SOURCE}/libs/iostreams/src/mapped_file.cpp
  DEFINE_PRIVATE
    BOOST_IOSTREAMS_USE_DEPRECATED
)

if(BOOST_IOSTREAMS_ENABLE_BZIP2)
  find_package(BZip2)
  if(BZIP2_FOUND)
    target_sources(Boost_iostreams PRIVATE
      ${BOOST_SOURCE}/libs/iostreams/src/bzip2.cpp
    )
    target_link_libraries(Boost_iostreams PRIVATE
      BZip2::BZip2
    )
  endif()
endif()

if(BOOST_IOSTREAMS_ENABLE_ZLIB)
  find_package(ZLIB)
  if(ZLIB_FOUND)
    target_sources(Boost_iostreams PRIVATE
      ${BOOST_SOURCE}/libs/iostreams/src/gzip.cpp
      ${BOOST_SOURCE}/libs/iostreams/src/zlib.cpp
    )
    target_link_libraries(Boost_iostreams PRIVATE
      ZLIB::ZLIB
    )
  endif()
endif()

_add_boost_test(
  NAME iostreams_test
  LINK
    Boost::iostreams
    Boost::filesystem
    Boost::regex
    Boost::unit_test_framework
  TESTS
    RUN ${BOOST_SOURCE}/libs/iostreams/test/array_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/auto_close_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/buffer_size_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/close_test.cpp
    #RUN ${BOOST_SOURCE}/libs/iostreams/test/code_converter_test.cpp
    #    ${BOOST_SOURCE}/libs/iostreams/test/detail/utf8_codecvt_facet.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/combine_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/compose_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/component_access_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/copy_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/counter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/direct_adapter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/example_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/execute_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/file_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/file_descriptor_test.cpp
    #RUN ${BOOST_SOURCE}/libs/iostreams/test/deprecated_file_descriptor_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/filtering_stream_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/finite_state_filter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/flush_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/grep_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/invert_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/line_filter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/mapped_file_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/path_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/newline_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/null_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/operation_sequence_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/pipeline_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/regex_filter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/restrict_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/seekable_file_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/seekable_filter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/sequence_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/slice_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/stdio_filter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/stream_offset_32bit_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/stream_offset_64bit_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/symmetric_filter_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/tee_test.cpp
    RUN ${BOOST_SOURCE}/libs/iostreams/test/wide_stream_test.cpp
    #RUN ${BOOST_SOURCE}/libs/iostreams/test/windos_pipe_test.cpp

    #TODO: Add large file tests
)

if(BZIP2_FOUND)
  _add_boost_test(
    NAME iostreams_test
    LINK
      Boost::iostreams
      Boost::filesystem
      Boost::unit_test_framework
    TESTS
      RUN ${BOOST_SOURCE}/libs/iostreams/test/bzip2_test.cpp
  )
endif()

if(ZLIB_FOUND)
  _add_boost_test(
    NAME iostreams_test
    LINK
      Boost::iostreams
      Boost::filesystem
      Boost::unit_test_framework
    TESTS
      RUN ${BOOST_SOURCE}/libs/iostreams/test/gzip_test.cpp
      RUN ${BOOST_SOURCE}/libs/iostreams/test/zlib_test.cpp
  )
endif()
