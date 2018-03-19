
/*
 *
 * A file to handle platform specific actions
 *
 */

#pragma once

// This is for offsets to functions in vtable on multiple platforms.
template<typename T>
constexpr T Platform(T _linux = 0, T _windows = 0, T _osx = 0) {
	#if defined(__linux__)
		return _linux;
	#elif defined(_WIN32)
		return _windows;
	#elif defined(__APPLE__)
		return _osx;
	#else
		#error "Unknown Platform"
	#endif
}

// String corrections
template<const char*>
constexpr const char* Platform(const char* _linux = "", const char* _windows = "", const char* _osx = "") {
	#if defined(__linux__)
		return _linux;
	#elif defined(_WIN32)
		return _windows;
	#elif defined(__APPLE__)
		return _osx;
	#else
		#error "Unknown Platform"
	#endif
}

// Since __has_include is new, we should make it not likely to break due to it not being defined
/*#define HAS_INCLUDE(x) ( \
	#if !defined(__has_include) \
		true \
	#else \
	 	__has_include(x))*/
