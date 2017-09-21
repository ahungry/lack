#!/bin/sh

[[ "x86_64 GNU/Linux" == $(uname -a | awk '{print $13 " " $14}') ]] || echo Build only tested with GNU/Linux so far, this may fail!

# Grab the Yue Release and store in a yue directory
[ -d yue ] || mkdir yue

cd yue

if [ ! -d include ]; then
    [ -e libyue.zip ] || wget https://github.com/yue/yue/releases/download/v0.2.0/libyue_v0.2.0_linux_x64.zip -O libyue.zip
    unzip libyue.zip
fi

# Return to top level
cd ..

# Make build dir for CMAKE
[ -d build ] || mkdir build

cd build && cmake -D CMAKE_BUILD_TYPE=Release .. && make
cd ..

[ -d bin ] || mkdir bin
mv build/lack bin/
