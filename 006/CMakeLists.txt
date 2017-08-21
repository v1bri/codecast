# CMake version 3.0 or higher is required for the Hunter package manager.
cmake_minimum_required(VERSION 3.0)

# Include the Hunter package manager module.
include("cmake/HunterGate.cmake")

# Ensure a reproducible build by locking down the Hunter package versions.
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.77.tar.gz"
    SHA1 "8898dd07756c408c63c63add903c8600c4cf4c0e")

project(codecast006)

# Tell Hunter which packages to pull in for our project.
hunter_add_package(Boost COMPONENTS system)
hunter_add_package(gflags)
hunter_add_package(glog)
hunter_add_package(nlohmann_json)

find_package(Boost CONFIG REQUIRED system)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(http_client
    http_client_main.cc)
target_compile_features(http_client
    PRIVATE cxx_lambdas cxx_nullptr)
target_link_libraries(http_client
    Boost::boost
    Boost::system
    glog::glog
    gflags
    nlohmann_json)
