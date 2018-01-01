
/*
 *
 * A file to handle platform specific actions
 *
 */

#pragma once

// TODO, make a better way to make something switch between platform an be compiled in, as in something like functions
#define PLATFORM_SPECIFIC(x1, x2, x3) (\
#if !defined(CMAKE_SYSTEM_NAME) || CMAKE_SYSTEM_NAME == Linux\
	x1\
#elif CMAKE_SYSTEM_NAME == Windows\
	x2\
#elif CMAKE_SYSTEM_NAME == OSX\
  x3\
#else\
	-1\
#endif\
)
