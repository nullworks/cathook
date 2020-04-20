_add_boost_lib(
  NAME timer
  SOURCES
    ${BOOST_SOURCE}/libs/timer/src/auto_timers_construction.cpp
    ${BOOST_SOURCE}/libs/timer/src/cpu_timer.cpp
  DEFINE_PRIVATE
    BOOST_TIMER_STATIC_LINK=1
  LINK
    Boost::chrono
)
