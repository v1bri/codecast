# CMake looks for build instructions in a file called CMakeLists.txt. Always
# start the top level CMakeLists.txt with the minimum required CMake version.
# This hints to the build system and future developers which features are
# currently in use.

# CMake version 3.0 or higher is required for the Hunter package manager.
cmake_minimum_required(VERSION 3.0)

# Include the Hunter package manager module.
include("cmake/HunterGate.cmake")

# Ensure a reproducible build by locking down the Hunter package versions.
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.19.32.tar.gz"
  SHA1 "a6ed6615c08e37b929b134f8c8e2e7392eb63f13")

# Pick a good name for the project, but you can always change it later.
project(codecast002)

# Tell Hunter which packages to pull in for our app. Each one has a helpful
# entry on Hunter's wiki that indicates how to include it.
hunter_add_package(gflags)
hunter_add_package(glog)

find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)

# An executable target will create a binary for our new app.
add_executable(codecast002
  codecast002_main.cc)
target_link_libraries(codecast002
  glog::glog
  gflags)

# CMake provides native support for C++11 (or greater) language features. Try to
# avoid altering CMAKE_CXX_FLAGS or similar variables if possible! The full list
# is in the CMake documentation.
target_compile_features(codecast002
  PRIVATE cxx_nullptr)
install(TARGETS codecast002 DESTINATION bin)
