# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.10)
project (libnullnexus)

add_library(libnullnexus INTERFACE)

# Thread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(libnullnexus INTERFACE Threads::Threads)

# Boost
find_package(Boost)

target_include_directories(libnullnexus INTERFACE include/)
