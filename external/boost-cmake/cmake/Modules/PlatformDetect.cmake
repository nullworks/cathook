# Basic platform detection
include(CheckPreprocessor)
check_preprocessor(USE_LINUX __linux__)
check_preprocessor(USE_WINDOWS _WIN32)
check_preprocessor(USE_APPLE __APPLE__)
check_preprocessor(USE_ANDROID __ANDROID__)
check_preprocessor(USE_FREEBSD __FreeBSD__)
