

if(USE_WINDOWS)
  set(SERIALIZATION_WIN_SRCS
    ${BOOST_SOURCE}/libs/serialization/src/basic_text_wiprimitive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_text_woprimitive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/text_wiarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/text_woarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_wgrammar.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_wiarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_woarchive.cpp
  )
  set(CXXFLAGS "/Gy")
endif()

_add_boost_lib(
  NAME serialization
  SOURCES
    ${SERIALIZATION_WIN_SRCS}
    ${BOOST_SOURCE}/libs/serialization/src/archive_exception.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_archive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_iarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_iserializer.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_oarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_oserializer.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_pointer_iserializer.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_pointer_oserializer.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_serializer_map.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_text_iprimitive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_text_oprimitive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_xml_archive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/basic_xml_grammar.ipp
    ${BOOST_SOURCE}/libs/serialization/src/binary_iarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/binary_oarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/binary_wiarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/binary_woarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/codecvt_null.cpp
    ${BOOST_SOURCE}/libs/serialization/src/extended_type_info.cpp
    ${BOOST_SOURCE}/libs/serialization/src/extended_type_info_no_rtti.cpp
    ${BOOST_SOURCE}/libs/serialization/src/extended_type_info_typeid.cpp
    ${BOOST_SOURCE}/libs/serialization/src/polymorphic_iarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/polymorphic_oarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/stl_port.cpp
    ${BOOST_SOURCE}/libs/serialization/src/text_iarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/text_oarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/utf8_codecvt_facet.cpp
    ${BOOST_SOURCE}/libs/serialization/src/void_cast.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_archive_exception.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_grammar.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_iarchive.cpp
    ${BOOST_SOURCE}/libs/serialization/src/xml_oarchive.cpp
  CXXFLAGS_PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Gy>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden -fvisibility-inlines-hidden -ftemplate-depth-255>
)
