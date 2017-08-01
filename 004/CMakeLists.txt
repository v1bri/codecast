# CMake version 3.0 or higher is required for the Hunter package manager.
cmake_minimum_required(VERSION 3.0)

# Include the Hunter package manager module.
include("cmake/HunterGate.cmake")

# Ensure a reproducible build by locking down the Hunter package versions.
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.55.tar.gz"
    SHA1 "fc4a3e21087b10708d45d503e4dc9e3aa228adf1")

project(codecast004)

# Tell Hunter which packages to pull in for our project.
hunter_add_package(Boost COMPONENTS system)
hunter_add_package(gflags)
hunter_add_package(glog)
hunter_add_package(nlohmann_json)

find_package(Boost CONFIG REQUIRED system)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(resolver
    resolver_main.cc)
target_compile_features(resolver
    PRIVATE cxx_lambdas)
target_link_libraries(resolver
    Boost::boost
    Boost::system
    glog::glog
    gflags
    nlohmann_json)
