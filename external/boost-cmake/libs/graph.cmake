_add_boost_lib(
  NAME graph
  SOURCES
    ${BOOST_SOURCE}/libs/graph/src/graphml.cpp
    ${BOOST_SOURCE}/libs/graph/src/read_graphviz_new.cpp
  LINK Boost::regex
)
