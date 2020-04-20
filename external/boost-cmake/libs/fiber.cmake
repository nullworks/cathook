_add_boost_lib(
  NAME fiber
  SOURCES
    ${BOOST_SOURCE}/libs/fiber/src/
    ${BOOST_SOURCE}/libs/fiber/src/algo/algorithm.cpp
    ${BOOST_SOURCE}/libs/fiber/src/algo/round_robin.cpp
    ${BOOST_SOURCE}/libs/fiber/src/algo/shared_work.cpp
    ${BOOST_SOURCE}/libs/fiber/src/algo/work_stealing.cpp
    ${BOOST_SOURCE}/libs/fiber/src/barrier.cpp
    ${BOOST_SOURCE}/libs/fiber/src/condition_variable.cpp
    ${BOOST_SOURCE}/libs/fiber/src/context.cpp
    ${BOOST_SOURCE}/libs/fiber/src/fiber.cpp
    ${BOOST_SOURCE}/libs/fiber/src/future.cpp
    ${BOOST_SOURCE}/libs/fiber/src/mutex.cpp
    ${BOOST_SOURCE}/libs/fiber/src/properties.cpp
    ${BOOST_SOURCE}/libs/fiber/src/recursive_mutex.cpp
    ${BOOST_SOURCE}/libs/fiber/src/recursive_timed_mutex.cpp
    ${BOOST_SOURCE}/libs/fiber/src/timed_mutex.cpp
    ${BOOST_SOURCE}/libs/fiber/src/scheduler.cpp
  DEFINE_PRIVATE
    BOOST_FIBERS_SOURCE=1
  LINK
    Boost::context
)
set_target_properties(Boost_fiber PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
)

_add_boost_test(
  NAME fiber_test
  LINK
    Boost::fiber
    Boost::thread
    Boost::unit_test_framework
  TESTS
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_async_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_async_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_barrier_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_barrier_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_buffered_channel_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_buffered_channel_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_condition_mt_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_condition_mt_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_condition_variable_any_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_condition_variable_any_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_condition_variable_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_condition_variable_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_fiber_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_fiber_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_fss_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_fss_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_future_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_future_mt_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_future_mt_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_future_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_mutex_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_mutex_mt_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_mutex_mt_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_mutex_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_packaged_task_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_packaged_task_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_promise_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_promise_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_shared_future_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_shared_future_post.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_unbuffered_channel_dispatch.cpp
    RUN ${BOOST_SOURCE}/libs/fiber/test/test_unbuffered_channel_post.cpp
)
