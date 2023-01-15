#!/bin/bash

usage() { echo "usage: $0 [-a|c {a} (all|clean)] [-r|d {d} (release|debug)] [-g <cmake-generator> {Ninja}]" 2>&1; exit 0; }

BUILD_TYPE="Debug"
GENERATOR="Ninja"
TARGET="all"
ACTION="Build"

while getopts ":acrdg:h" arg; do
    case "${arg}" in
        a)
            TARGET="all"
            ACTION="Build"
            ;;
        c)
            TARGET="clean"
            ACTION="Clean"
            ;;
        r)
            BUILD_TYPE="Release"
            ;;
        d)
            BUILD_TYPE="Debug"
            ;;
        g)
            GENERATOR=${OPTARG}
            ;;
        h|*)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${GENERATOR}" ]; then
    usage
fi

if [ ${TARGET} = "all" ]; then
    if [ -d ${PWD}/build ]; then
        rm -rf build/{.[!.]*,*}
    fi
    echo "Configuring Event Loop in ${BUILD_TYPE} mode..."
    cmake -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE} -S . -B ${PWD}/build -G "${GENERATOR}"
fi

echo "${ACTION}ing Event Loop..."
if [ ! -z "$(ls -A ${PWD}/build)" ]; then
    cmake --build ${PWD}/build --config ${BUILD_TYPE} --target ${TARGET}
fi

if [ ${TARGET} = "clean" ]; then
    if [ -d ${PWD}/build ]; then
        rm -rf build/{.[!.]*,*}
    fi
    if [ -d ${PWD}/lib ]; then
        rm -rf lib/*
    fi
fi