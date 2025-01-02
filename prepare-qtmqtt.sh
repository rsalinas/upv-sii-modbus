#!/bin/bash
set -eux

export PATH=$PATH:$HOME/Qt/6.8.1/gcc_64/bin

rm -rf qtmqtt
git clone https://github.com/qt/qtmqtt.git --branch 6.8.1

trap 'rm -rf qtmqtt qtmqtt-build' EXIT

cmake -S qtmqtt -B qtmqtt-build
cmake --build qtmqtt-build --target install -- -j$(nproc)

