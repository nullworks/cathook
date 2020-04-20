# Detect Boost version
file(STRINGS "${BOOST_SOURCE}/boost/version.hpp" boost_version_raw
  REGEX "define BOOST_VERSION "
)
string(REGEX MATCH "[0-9]+" boost_version_raw "${boost_version_raw}")
math(EXPR BOOST_VERSION_MAJOR "${boost_version_raw} / 100000")
math(EXPR BOOST_VERSION_MINOR "${boost_version_raw} / 100 % 1000")
math(EXPR BOOST_VERSION_PATCH "${boost_version_raw} % 100")
set(BOOST_VERSION "${BOOST_VERSION_MAJOR}.${BOOST_VERSION_MINOR}.${BOOST_VERSION_PATCH}")
