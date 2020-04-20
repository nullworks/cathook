_add_boost_lib(
  NAME date_time
  SOURCES
    ${BOOST_SOURCE}/libs/date_time/src/gregorian/greg_month.cpp
    ${BOOST_SOURCE}/libs/date_time/src/gregorian/greg_weekday.cpp
    ${BOOST_SOURCE}/libs/date_time/src/gregorian/date_generators.cpp
  DEFINE_PRIVATE
    BOOST_DATE_TIME_STATIC_LINK
    DATE_TIME_INLINE
)

_add_boost_test(
  NAME date_time_test
  DEFINE
    BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
  LINK
    Boost::date_time
  TESTS
    # core suite
    RUN ${BOOST_SOURCE}/libs/date_time/test/testint_adapter.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/testtime_resolution_traits.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/testwrapping_int.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/testconstrained_value.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/testgregorian_calendar.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/testgeneric_period.cpp

    # gregorian suite
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testdate.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testdate_duration.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testgreg_durations.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testperiod.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testdate_iterator.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testformatters.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testdate_facet_new.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testdate_input_facet.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testgenerators.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testgreg_cal.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testgreg_day.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testgreg_month.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/gregorian/testgreg_year.cpp

    # posixtime
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testfiletime_functions.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testlocal_adjustor.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testc_local_adjustor.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testclock.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testdst_rules.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testduration.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testiterator.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testparse_time.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testtime_period.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testtime.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testmicrosec_time_clock.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testtime_formatters.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testgreg_duration_operators.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testtime_facet.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/posix_time/testtime_input_facet.cpp

    # localtime suite
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testdst_transition_day_rule.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testcustom_time_zone.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testposix_time_zone.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testwcustom_time_zone.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testwposix_time_zone.cpp
    # Uses data file, which may have line endings and fail the test
    # RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testtz_database.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testlocal_time.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testlocal_time_iterator.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testlocal_time_period.cpp
    # Uses data file, which may have line endings and fail the test
    # RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testlocal_time_facet.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testlocal_time_input_facet.cpp
    RUN ${BOOST_SOURCE}/libs/date_time/test/local_time/testclocks.cpp
)

# TODO: Add missing serialization tests
