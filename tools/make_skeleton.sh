#!/bin/bash
# Simple script to initialize a codecast project with skeleton CMakeLists.txt
# and skeleton C++ main file.

JQ=$(which jq)
if [[ -z "$JQ" ]]; then
    echo "Please install jq before running this script."
    exit 1
fi

CURL=$(which curl)
if [[ -z "$CURL" ]]; then
    echo "Please install curl before running this script."
    exit 1
fi

if [[ "$#" -ne 3 ]]; then
    echo "Usage:"
    echo "    $0 <path_to_generated_files> <project_name> <initial_target_name>"
    exit 1
fi

DEST_PATH=$1
PROJECT_NAME=$2
TARGET_NAME=$3

# Create cmake module folder and download the HunterGate module.
mkdir -p "$DEST_PATH/cmake" || exit 1
$CURL -s -L -o "$DEST_PATH/cmake/HunterGate.cmake" https://github.com/hunter-packages/gate/raw/master/cmake/HunterGate.cmake

# Create the skeleton CMakeLists.txt file.
echo "# CMake version 3.0 or higher is required for the Hunter package manager." > $DEST_PATH/CMakeLists.txt
echo "cmake_minimum_required(VERSION 3.0)" >> $DEST_PATH/CMakeLists.txt
echo >> $DEST_PATH/CMakeLists.txt
echo "# Include the Hunter package manager module." >> $DEST_PATH/CMakeLists.txt
echo "include(\"cmake/HunterGate.cmake\")" >> $DEST_PATH/CMakeLists.txt
echo >> $DEST_PATH/CMakeLists.txt
echo "# Ensure a reproducible build by locking down the Hunter package versions." >> $DEST_PATH/CMakeLists.txt

# Something is stripping all but one space between words when curl's output is
# assigned to a variable instead of piped to jq. So I'm resorting to sending the
# command output directly to file.
$CURL -s https://api.github.com/repos/ruslo/hunter/releases/latest | $JQ -r '.body | gsub("```cmake\r\n"; "") | gsub("\r\n\\)"; ")") | gsub("```\r\n"; "") | gsub("\r\n"; "\n")' >> $DEST_PATH/CMakeLists.txt
echo "project($PROJECT_NAME)" >> $DEST_PATH/CMakeLists.txt
echo >> $DEST_PATH/CMakeLists.txt
echo "# Tell Hunter which packages to pull in for our project." >> $DEST_PATH/CMakeLists.txt
echo "hunter_add_package(gflags)" >> $DEST_PATH/CMakeLists.txt
echo "hunter_add_package(glog)" >> $DEST_PATH/CMakeLists.txt
echo >> $DEST_PATH/CMakeLists.txt
echo "find_package(gflags CONFIG REQUIRED)" >> $DEST_PATH/CMakeLists.txt
echo "find_package(glog CONFIG REQUIRED)" >> $DEST_PATH/CMakeLists.txt
echo >> $DEST_PATH/CMakeLists.txt
echo "add_executable($TARGET_NAME" >> $DEST_PATH/CMakeLists.txt
echo "    ${TARGET_NAME}_main.cc)" >> $DEST_PATH/CMakeLists.txt
echo "target_link_libraries($TARGET_NAME" >> $DEST_PATH/CMakeLists.txt
echo "    glog::glog" >> $DEST_PATH/CMakeLists.txt
echo "    gflags)" >> $DEST_PATH/CMakeLists.txt

# Create the skeleton main file.
echo "#include <gflags/gflags.h>" > $DEST_PATH/${TARGET_NAME}_main.cc
echo "#include <glog/logging.h>" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo  >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "int main(int argc, char* argv[]) {" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "    gflags::SetUsageMessage(\"\");" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "    gflags::SetVersionString(\"\");" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "    gflags::ParseCommandLineFlags(&argc, &argv, true);" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "    google::InitGoogleLogging(argv[0]);" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "    google::InstallFailureSignalHandler();" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "    return 0;" >> $DEST_PATH/${TARGET_NAME}_main.cc
echo "}" >> $DEST_PATH/${TARGET_NAME}_main.cc
