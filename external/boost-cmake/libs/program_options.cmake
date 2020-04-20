_add_boost_lib(
  NAME program_options
  SOURCES
    ${BOOST_SOURCE}/libs/program_options/src/cmdline.cpp
    ${BOOST_SOURCE}/libs/program_options/src/config_file.cpp
    ${BOOST_SOURCE}/libs/program_options/src/options_description.cpp
    ${BOOST_SOURCE}/libs/program_options/src/parsers.cpp
    ${BOOST_SOURCE}/libs/program_options/src/variables_map.cpp
    ${BOOST_SOURCE}/libs/program_options/src/value_semantic.cpp
    ${BOOST_SOURCE}/libs/program_options/src/positional_options.cpp
    ${BOOST_SOURCE}/libs/program_options/src/utf8_codecvt_facet.cpp
    ${BOOST_SOURCE}/libs/program_options/src/convert.cpp
    ${BOOST_SOURCE}/libs/program_options/src/winmain.cpp
    ${BOOST_SOURCE}/libs/program_options/src/split.cpp
)

_add_boost_test(
  NAME program_options_test
  LINK
    Boost::program_options
  TESTS
    RUN ${BOOST_SOURCE}/libs/program_options/test/options_description_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/variable_map_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/cmdline_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/positional_options_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/unicode_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/winmain.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/exception_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/split_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/unrecognized_test.cpp
    RUN ${BOOST_SOURCE}/libs/program_options/test/exception_txt_test.cpp
)
