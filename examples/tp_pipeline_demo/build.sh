#!/bin/bash

buildType=$1

if [ "debug" = "${buildType}" ]; then
    echo "build debug"
elif [ "release" = "${buildType}" ]; then
    echo "build release"
else
    echo "unkown build type ${buildType}"
    exit 1
fi

rm -r ./build/*
mkdir -p ./build && cd build && cmake -DCMAKE_BUILD_TYPE=${buildType} ../ && make
