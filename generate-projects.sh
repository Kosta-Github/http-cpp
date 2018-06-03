#!/usr/bin/env bash
#
# Usage:
# generate_projects [build output directory]
#

function checkExit {
  if [[ $1 != 0 ]]; then
      echo $2
    exit $1
  fi
}


PLATFORM="unknown"
if [[ "$(uname)" == "Linux" ]]; then
  PLATFORM='Linux'
elif [[ "$(uname)" == "Darwin" ]]; then
  PLATFORM="MacOSX"
else
  echo "Unknown platform!"
  exit 1
fi


# Paths and Defines
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "$#" == "0" ]; then
    # No build directory was given, generate build files in '/build'
    OUTPUT_DIR="$BASE_DIR/build/$PLATFORM"
else
    # Use build directory passed as first parameter
    OUTPUT_DIR=$1
fi

mkdir -p $OUTPUT_DIR

pushd "$BASE_DIR"
  cmake -H. -B$OUTPUT_DIR/Debug -DCMAKE_BUILD_TYPE=Debug
  checkExit $? "Makefile generation for 'Debug' build failed."

  cmake -H. -B$OUTPUT_DIR/RelWithDebInfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
  checkExit $? "Makefile generation for 'RelWithDebInfo' build failed."

  if [[ "$PLATFORM" == "MacOSX" ]]; then
    cmake -H. -B$OUTPUT_DIR/Xcode -G Xcode
    checkExit $? "Xcode project generation failed."
  fi
popd
