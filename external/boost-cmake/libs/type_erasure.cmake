_add_boost_lib(
  NAME type_erasure
  SOURCES
    ${BOOST_SOURCE}/libs/type_erasure/src/dynamic_binding.cpp
  LINK
    Boost::thread
)
