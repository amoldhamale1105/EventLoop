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
    mkdir -p "${COVERAGE_DIR}"

    OS="$(uname -s)"
    if [[ "${OS}" == MINGW* ]] || [[ "${OS}" == MSYS* ]] || [[ "${OS}" == CYGWIN* ]]; then
        # ── Windows: OpenCppCoverage ──────────────────────────────────────────
        # Requires OpenCppCoverage on PATH.
        # Install via: winget install OpenCppCoverage  OR  choco install opencppcoverage
        if ! command -v OpenCppCoverage &>/dev/null; then
            echo "WARNING: OpenCppCoverage not found on PATH. Skipping Windows coverage report."
            echo "Install it with: winget install OpenCppCoverage"
        else
            # Support both multi-config generators (MSVC) and single-config (Ninja/MinGW).
            # Multi-config puts binaries under build/tests/<BuildType>/; single-config does not.
            if [ -f "${BUILD_DIR}/tests/${BUILD_TYPE}/EventLoopTests.exe" ]; then
                TEST_EXE="${BUILD_DIR}/tests/${BUILD_TYPE}/EventLoopTests.exe"
                LIB_DLL="${BUILD_DIR}/${BUILD_TYPE}/EventLoop.dll"
            else
                TEST_EXE="${BUILD_DIR}/tests/EventLoopTests.exe"
                LIB_DLL="${BUILD_DIR}/EventLoop.dll"
            fi

            echo "Generating Windows coverage report with OpenCppCoverage..."
            OpenCppCoverage \
                --sources "${PWD}/src" \
                --sources "${PWD}/include" \
                --modules "${LIB_DLL}" \
                --export_type "html:${COVERAGE_DIR}/html" \
                --export_type "cobertura:${COVERAGE_DIR}/coverage.xml" \
                -- "${TEST_EXE}"

            echo "Coverage report generated at: ${COVERAGE_DIR}/html/index.html"
        fi
    else
        # ── Linux / macOS: lcov + genhtml ─────────────────────────────────────
        # --ignore-errors unused: suppresses harmless warning when _deps/ does not
        # exist (e.g. when gtest is installed system-wide instead of via FetchContent)
        lcov --directory "${BUILD_DIR}" --capture \
            --output-file "${COVERAGE_DIR}/coverage.info" \
            --rc branch_coverage=1 \
            --exclude '/usr/*' --exclude '*/tests/*' --exclude '*/_deps/*' \
            --ignore-errors mismatch,empty,unused \
            --quiet

        genhtml --output-directory "${COVERAGE_DIR}/html" \
                --title "Event Loop Code Coverage" \
                --prefix "${PWD}" --rc branch_coverage=1 --quiet \
                "${COVERAGE_DIR}/coverage.info"

        echo "Coverage report generated at: ${COVERAGE_DIR}/html/index.html"
    fi
fi

if [ ${TARGET} = "clean" ]; then
    if [ -d ${PWD}/build ]; then
        rm -rf build/{.[!.]*,*}
    fi
    if [ -d ${PWD}/lib ]; then
        rm -rf lib/*
    fi
fi
