#!/bin/bash

if [ "Darwin" == "$(uname)" ]; then
  OS="mac"
elif [[ "$(uname -a)" =~ "x86_64 GNU/Linux" ]]; then
  OS="linux"
else
  echo "OS not supported."
  exit 1
fi

URL="https://github.com/yue/yue/releases/download/v0.2.0/libyue_v0.2.0_${OS}_x64.zip"

# Grab the Yue Release and store in a yue directory
[ -d yue ] || mkdir yue

cd yue

if [ ! -d include ]; then
    echo "Downloading $URL"

    [ -e libyue.zip ] || wget $URL -O libyue.zip
    unzip libyue.zip
fi

# Return to top level
cd ..

# Make build dir for CMAKE
[ -d build ] || mkdir build

#cd build && cmake -D CMAKE_BUILD_TYPE=Release .. && make VERBOSE=1
cd build && cmake -D CMAKE_BUILD_TYPE=Debug .. && make VERBOSE=1
cd ..

[ -d bin ] || mkdir bin
cp build/lack bin/
