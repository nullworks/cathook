option(BOOST_LOG_WITHOUT_DEBUG_OUTPUT "Disable debug output backend in Boost Log" OFF)
option(BOOST_LOG_WITHOUT_EVENT_LOG "Disable event log backend in Boost Log" OFF)
option(BOOST_LOG_WITHOUT_SYSLOG "Disable syslog backend in Boost Log" OFF)

_add_boost_lib(
  NAME log
  SOURCES
    ${BOOST_SOURCE}/libs/log/src/attribute_name.cpp
    ${BOOST_SOURCE}/libs/log/src/attribute_set.cpp
    ${BOOST_SOURCE}/libs/log/src/attribute_value_set.cpp
    ${BOOST_SOURCE}/libs/log/src/code_conversion.cpp
    ${BOOST_SOURCE}/libs/log/src/core.cpp
    ${BOOST_SOURCE}/libs/log/src/record_ostream.cpp
    ${BOOST_SOURCE}/libs/log/src/severity_level.cpp
    ${BOOST_SOURCE}/libs/log/src/global_logger_storage.cpp
    ${BOOST_SOURCE}/libs/log/src/named_scope.cpp
    ${BOOST_SOURCE}/libs/log/src/process_name.cpp
    ${BOOST_SOURCE}/libs/log/src/process_id.cpp
    ${BOOST_SOURCE}/libs/log/src/thread_id.cpp
    ${BOOST_SOURCE}/libs/log/src/timer.cpp
    ${BOOST_SOURCE}/libs/log/src/exceptions.cpp
    ${BOOST_SOURCE}/libs/log/src/default_attribute_names.cpp
    ${BOOST_SOURCE}/libs/log/src/default_sink.cpp
    ${BOOST_SOURCE}/libs/log/src/text_ostream_backend.cpp
    ${BOOST_SOURCE}/libs/log/src/text_file_backend.cpp
    ${BOOST_SOURCE}/libs/log/src/text_multifile_backend.cpp
    ${BOOST_SOURCE}/libs/log/src/thread_specific.cpp
    ${BOOST_SOURCE}/libs/log/src/once_block.cpp
    ${BOOST_SOURCE}/libs/log/src/timestamp.cpp
    ${BOOST_SOURCE}/libs/log/src/threadsafe_queue.cpp
    ${BOOST_SOURCE}/libs/log/src/event.cpp
    ${BOOST_SOURCE}/libs/log/src/trivial.cpp
    ${BOOST_SOURCE}/libs/log/src/spirit_encoding.cpp
    ${BOOST_SOURCE}/libs/log/src/format_parser.cpp
    ${BOOST_SOURCE}/libs/log/src/date_time_format_parser.cpp
    ${BOOST_SOURCE}/libs/log/src/named_scope_format_parser.cpp
    ${BOOST_SOURCE}/libs/log/src/permissions.cpp
    ${BOOST_SOURCE}/libs/log/src/dump.cpp
  DEFINE_PRIVATE
    _STDC_CONSTANT_MACROS
    BOOST_SPIRIT_USE_PHOENIX_V3=1
    BOOST_THREAD_DONT_USE_CHRONO=1 
    BOOST_LOG_BUILDING_THE_LIB=1
    $<$<BOOL:${USE_WINDOWS}>:
      NOMINMAX
      WIN32_LEAN_AND_MEAN
      SECURITY_WIN32
      __USE_W32_SOCKETS
      BOOST_USE_WINDOWS_H
    >
  INCLUDE_PRIVATE
    ${BOOST_SOURCE}/libs/log/src
  LINK
    Boost::atomic
    Boost::date_time
    Boost::filesystem
    Boost::thread
    $<$<BOOL:${USE_WINDOWS}>:
      ws2_32
      mswsock
      advapi32
    >
)

try_compile(HAVE_PTHREAD_MUTEX_ROBUST
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${BOOST_SOURCE}/libs/log/config/pthread-mutex-robust/pthread_mutex_robust.cpp"
  COMPILE_DEFINITIONS _XOPEN_SOURCE=600 _GNU_SOURCE=1
)
if(HAVE_PTHREAD_MUTEX_ROBUST)
  target_compile_definitions(Boost_log PRIVATE BOOST_LOG_HAS_PTHREAD_MUTEX_ROBUST)
endif()

try_compile(HAVE_ATOMIC_INT32
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${BOOST_SOURCE}/libs/log/config/atomic-int32/atomic_int32.cpp"
  LINK_LIBRARIES Boost::boost
)
if(NOT HAVE_ATOMIC_INT32)
  target_compile_definitions(Boost_log PRIVATE BOOST_LOG_WITHOUT_IPC)
else()
  if(USE_WINDOWS)
    target_sources(Boost_log PRIVATE
      ${BOOST_SOURCE}/libs/log/src/windows/object_name.cpp
      ${BOOST_SOURCE}/libs/log/src/windows/mapped_shared_memory.cpp
      ${BOOST_SOURCE}/libs/log/src/windows/ipc_sync_wrappers.cpp
      ${BOOST_SOURCE}/libs/log/src/windows/ipc_reliable_message_queue.cpp
    )
    target_link_libraries(Boost_log PRIVATE secur32)
  else()
    target_sources(Boost_log PRIVATE
      ${BOOST_SOURCE}/libs/log/src/posix/object_name.cpp
      ${BOOST_SOURCE}/libs/log/src/posix/ipc_reliable_message_queue.cpp
    )
  endif()
endif()

try_compile(HAVE_NATIVE_SYSLOG
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${BOOST_SOURCE}/libs/log/config/native-syslog/native_syslog.cpp"
)
if(NOT BOOST_LOG_WITHOUT_SYSLOG AND HAVE_NATIVE_SYSLOG)
  target_compile_definitions(Boost_log PRIVATE BOOST_LOG_USE_NATIVE_SYSLOG)
  target_sources(Boost_log PRIVATE
    ${BOOST_SOURCE}/libs/log/src/syslog_backend.cpp
  )
else()
  target_compile_definitions(Boost_log PRIVATE BOOST_LOG_WITHOUT_SYSLOG)
endif()

find_program(MC_EXECUTABLE mc
  DOC "Message Compiler executable"
)
if(NOT BOOST_LOG_WITHOUT_EVENT_LOG AND MC_EXECUTABLE AND FALSE)
  # TODO: Enable this on Windows
  # windows/simple_event_log.mc should be compiled with mc.exe
  target_sources(Boost_log PRIVATE
    ${BOOST_SOURCE}/libs/log/src/windows/event_log_backend.cpp
  )
  target_link_libraries(Boost_log PRIVATE psapi)
else()
  target_compile_definitions(Boost_log PRIVATE BOOST_LOG_WITHOUT_EVENT_LOG)
endif()

if(USE_WINDOWS AND NOT BOOST_LOG_WITHOUT_DEBUG_OUTPUT)
  target_sources(Boost_log PRIVATE
    ${BOOST_SOURCE}/libs/log/src/windows/debug_output_backend.cpp
  )
else()
  target_compile_definitions(Boost_log PRIVATE BOOST_LOG_WITHOUT_DEBUG_OUTPUT)
endif()

#TODO: Handle SSSE3 and AVX2 optimizations

_add_boost_lib(
  NAME log_setup
  SOURCES
    ${BOOST_SOURCE}/libs/log/src/setup/default_filter_factory.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/default_filter_factory.hpp
    ${BOOST_SOURCE}/libs/log/src/setup/default_formatter_factory.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/default_formatter_factory.hpp
    ${BOOST_SOURCE}/libs/log/src/setup/filter_parser.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/formatter_parser.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/init_from_settings.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/init_from_stream.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/matches_relation_factory.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/parser_utils.cpp
    ${BOOST_SOURCE}/libs/log/src/setup/parser_utils.hpp
    ${BOOST_SOURCE}/libs/log/src/setup/settings_parser.cpp
  DEFINE_PRIVATE
    BOOST_LOG_SETUP_BUILDING_THE_LIB=1
  INCLUDE_PRIVATE
    ${BOOST_SOURCE}/libs/log/src
  LINK
    Boost::log
)

add_library(Boost_log_common INTERFACE)
target_include_directories(Boost_log_common INTERFACE ${BOOST_SOURCE}/libs/log/test/common)

_add_boost_test(
  NAME log_test
  LINK
    Boost::date_time
    Boost::filesystem
    Boost::log
    Boost::log_setup
    Boost::regex
    Boost::thread
    Boost::unit_test_framework
    Boost_log_common
  TESTS
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_attribute_set.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_attribute_set_ticket11106.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_attribute_value_impl.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_attribute_value_set.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_attribute_value_set_ticket11190.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_function.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_named_scope.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_sets_insertion_lookup.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/attr_value_visitation.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/core.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_attr.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_has_attr.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_matches_boost_regex.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_matches_spirit_classic.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_matches_spirit_qi.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_matches_std_regex.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/filt_matches_xpressive.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_attr.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_c_decor.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_char_decor.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_csv_decor.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_date_time.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_format.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_if.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_max_size_decor.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_message.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_named_scope.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_to_log_manip.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/form_xml_decor.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/sink_text_ipc_mq_backend.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/src_record_ostream.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_dynamic_type_disp.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_exception_handler.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_formatting_ostream.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_ipc_object_name.cpp
    # RUN ${BOOST_SOURCE}/libs/log/test/run/util_ipc_reliable_mq.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_manip_add_value.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_manip_dump.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_manip_to_log.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_once_block.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_static_type_disp.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_stp_filter_parser.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_stp_formatter_parser.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_stp_settings_parser.cpp
    RUN ${BOOST_SOURCE}/libs/log/test/run/util_string_literal.cpp
)
