#!/bin/bash

usage() { echo "usage: $0 [-a|c {a} (all|clean)] [-r|d {d} (release|debug)] [-g <cmake-generator> {Ninja}] [-t (test and coverage)]" 2>&1; exit 0; }

BUILD_TYPE="Debug"
GENERATOR="Ninja"
TARGET="all"
ACTION="Build"

while getopts ":acrdtg:h" arg; do
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
        t)
            COVERAGE="ON"
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
    cmake -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE} -DTEST_COVERAGE:BOOL=${COVERAGE} -S . -B ${PWD}/build -G "${GENERATOR}"
fi

echo "${ACTION}ing Event Loop..."
if [ ! -z "$(ls -A ${PWD}/build)" ]; then
    cmake --build ${PWD}/build --config ${BUILD_TYPE} --target ${TARGET}
fi

# Run tests and generate coverage report if coverage is enabled
if [ "${COVERAGE}" = "ON" ]; then
    echo "Running unit tests..."
    BUILD_DIR="$(pwd)/build"
    (cd "${BUILD_DIR}" && ctest --output-on-failure) || { echo "Tests failed."; exit 1; }
    
    echo "Tests passed. Generating coverage report..."
    COVERAGE_DIR="${BUILD_DIR}/coverage"
    mkdir -p ${COVERAGE_DIR}
    
    lcov --directory "${BUILD_DIR}" --capture --output-file ${COVERAGE_DIR}/coverage.info \
        --rc branch_coverage=1 --exclude '/usr/*' --exclude '*/tests/*' --exclude '*/_deps/*' \
        --quiet
    
    genhtml --output-directory ${COVERAGE_DIR}/html --title "Event Loop Code Coverage" \
            --prefix "${PWD}" --rc branch_coverage=1 --quiet ${COVERAGE_DIR}/coverage.info
    
    echo "Coverage report generated at: ${COVERAGE_DIR}/html/index.html"
fi

if [ ${TARGET} = "clean" ]; then
    if [ -d ${PWD}/build ]; then
        rm -rf build/{.[!.]*,*}
    fi
    if [ -d ${PWD}/lib ]; then
        rm -rf lib/*
    fi
fi