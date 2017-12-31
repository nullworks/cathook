# UNFINISHED - DO NOT USE.

cmake_minimum_required ( VERSION 3.0 )
project ( cathook )

set (SOURCESDK "source-sdk-2013-headers")

include_directories ( . include ucccccp SYSTEM ${SOURCESDK} ${SOURCESDK}/public ${SOURCESDK}/mathlib ${SOURCESDK}/common ${SOURCESDK}/public/tier0 ${SOURCESDK}/public/tier1)



add_subdirectory (src)