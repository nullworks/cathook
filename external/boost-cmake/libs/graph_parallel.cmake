if(NOT TARGET Boost_mpi)
  return()
endif()

_add_boost_lib(
  NAME graph_parallel
  SOURCES
    ${BOOST_SOURCE}/libs/graph_parallel/src/mpi_process_group.cpp
    ${BOOST_SOURCE}/libs/graph_parallel/src/tag_allocator.cpp
  LINK
    Boost::mpi
)
