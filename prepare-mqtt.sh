#! /bin/bash 
set -eu 

git clone --branch v6.8.2 https://github.com/qt/qtmqtt.git
cd qtmqtt
~/Qt/6.8.2/gcc_64/bin/qt-cmake .
cmake --build . --parallel
cmake --install .

