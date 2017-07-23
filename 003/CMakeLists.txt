# CMake version 3.0 or higher is required for the Hunter package manager.
cmake_minimum_required(VERSION 3.0)

# Include the Hunter package manager module.
include("cmake/HunterGate.cmake")

# Ensure a reproducible build by locking down the Hunter package versions.
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.38.tar.gz"
    SHA1 "3154fdc77457d6ebec50ba2862ebc341fed25fed")

project(codecast003)

# Tell Hunter which packages to pull in for our project.
hunter_add_package(gflags)
hunter_add_package(glog)
hunter_add_package(Boost COMPONENTS system)

find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system)

add_executable(codecast003_io_service
    codecast003_io_service_main.cc)
target_link_libraries(codecast003_io_service
    Boost::boost
    Boost::system
    glog::glog
    gflags)
target_compile_features(codecast003_io_service
    PRIVATE cxx_lambdas)

add_executable(codecast003_counter
    codecast003_counter_main.cc)
target_link_libraries(codecast003_counter
    Boost::boost
    Boost::system
    glog::glog
    gflags)
target_compile_features(codecast003_counter
    PRIVATE cxx_lambdas)

add_executable(codecast003_resolver
    codecast003_resolver_main.cc)
target_link_libraries(codecast003_resolver
    Boost::boost
    Boost::system
    glog::glog
    gflags)
target_compile_features(codecast003_resolver
    PRIVATE cxx_lambdas)

