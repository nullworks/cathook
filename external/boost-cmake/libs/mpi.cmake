find_package(MPI QUIET)

if(NOT MPI_CXX_FOUND)
  return()
endif()

_add_boost_lib(
  NAME mpi
  SOURCES
    ${BOOST_SOURCE}/libs/mpi/src/content_oarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/environment.cpp
    ${BOOST_SOURCE}/libs/mpi/src/exception.cpp
    ${BOOST_SOURCE}/libs/mpi/src/graph_communicator.cpp
    ${BOOST_SOURCE}/libs/mpi/src/group.cpp
    ${BOOST_SOURCE}/libs/mpi/src/intercommunicator.cpp
    ${BOOST_SOURCE}/libs/mpi/src/mpi_datatype_cache.cpp
    ${BOOST_SOURCE}/libs/mpi/src/mpi_datatype_oarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/packed_iarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/packed_oarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/packed_skeleton_iarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/packed_skeleton_oarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/point_to_point.cpp
    ${BOOST_SOURCE}/libs/mpi/src/request.cpp
    ${BOOST_SOURCE}/libs/mpi/src/text_skeleton_oarchive.cpp
    ${BOOST_SOURCE}/libs/mpi/src/timer.cpp
  DEFINE_PRIVATE
    BOOST_MPI_SOURCE=1
  LINK
    Boost::serialization
)
target_include_directories(Boost_mpi PUBLIC ${MPI_CXX_INCLUDE_PATH})
target_link_libraries(Boost_mpi PUBLIC ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
