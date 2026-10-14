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
    if [ "${TARGET}" == "all" ]; then
        cmake --build ${PWD}/build --config ${BUILD_TYPE}
    else
        cmake --build ${PWD}/build --config ${BUILD_TYPE} --target ${TARGET}
    fi
fi

# Run tests and generate coverage report if coverage is enabled
if [ "${COVERAGE}" = "ON" ]; then
    echo "Running unit tests..."
    BUILD_DIR="$(pwd)/build"
    (cd "${BUILD_DIR}" && ctest --build-config "${BUILD_TYPE}" --output-on-failure) || { echo "Tests failed."; exit 1; }

    echo "Tests passed. Generating coverage report..."
    COVERAGE_DIR="${BUILD_DIR}/coverage"
    mkdir -p "${COVERAGE_DIR}"

    OS="$(uname -s)"
    if [[ "${OS}" == MINGW* ]] || [[ "${OS}" == MSYS* ]] || [[ "${OS}" == CYGWIN* ]]; then
        COMPILER_ID="$(sed -n 's/^set(CMAKE_CXX_COMPILER_ID "\(.*\)")/\1/p' "${BUILD_DIR}"/CMakeFiles/*/CMakeCXXCompiler.cmake | head -n 1)"
        COMPILER_FRONTEND="$(sed -n 's/^set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT "\(.*\)")/\1/p' "${BUILD_DIR}"/CMakeFiles/*/CMakeCXXCompiler.cmake | head -n 1)"

        if [ "${COMPILER_ID}" != "MSVC" ] && [ "${COMPILER_FRONTEND}" != "MSVC" ]; then
            echo "ERROR: Windows coverage requires an MSVC-compatible compiler; OpenCppCoverage needs PDB symbols."
            exit 1
        fi
        if ! command -v OpenCppCoverage &>/dev/null; then
            echo "ERROR: OpenCppCoverage not found on PATH. Install it with: winget install OpenCppCoverage"
            exit 1
        fi

        # Locate test executables for both multi- and single-config generators.
        if [ -f "${BUILD_DIR}/tests/${BUILD_TYPE}/EventLoopTests.exe" ]; then
            TEST_DIR="${BUILD_DIR}/tests/${BUILD_TYPE}"
            CONFIG_DIR="${BUILD_DIR}/${BUILD_TYPE}"
        else
            TEST_DIR="${BUILD_DIR}/tests"
            CONFIG_DIR="${BUILD_DIR}"
        fi

        TEST_EXE="${TEST_DIR}/EventLoopTests.exe"
        THREAD_TEST_EXE="${TEST_DIR}/EventLoopThreadSafetyTests.exe"
        LIB_DLL=""
        for candidate in \
            "${CONFIG_DIR}/EventLoop.dll" \
            "${BUILD_DIR}/EventLoop.dll" \
            "${PWD}/lib/EventLoop.dll"; do
            if [ -f "${candidate}" ]; then
                LIB_DLL="${candidate}"
                break
            fi
        done

        if [ ! -f "${TEST_EXE}" ] || [ ! -f "${THREAD_TEST_EXE}" ] || [ -z "${LIB_DLL}" ]; then
            echo "ERROR: Could not locate both test executables and the EventLoop DLL."
            echo "Expected tests under: ${TEST_DIR}"
            exit 1
        fi

        echo "Generating Windows coverage report with OpenCppCoverage..."
        COVERAGE_DATA="${COVERAGE_DIR}/EventLoopBasicTest.cov"
        PATH="$(dirname "${LIB_DLL}"):${PATH}" OpenCppCoverage \
            --sources "$(cygpath -w "${PWD}/src")" \
            --sources "$(cygpath -w "${PWD}/include")" \
            --modules "$(cygpath -w "${LIB_DLL}")" \
            --export_type "binary:$(cygpath -w "${COVERAGE_DATA}")" \
            -- "$(cygpath -w "${TEST_EXE}")" || { echo "Coverage generation failed for EventLoopTests."; exit 1; }

        PATH="$(dirname "${LIB_DLL}"):${PATH}" OpenCppCoverage \
            --sources "$(cygpath -w "${PWD}/src")" \
            --sources "$(cygpath -w "${PWD}/include")" \
            --modules "$(cygpath -w "${LIB_DLL}")" \
            --input_coverage "$(cygpath -w "${COVERAGE_DATA}")" \
            --export_type "html:$(cygpath -w "${COVERAGE_DIR}/html")" \
            --export_type "cobertura:$(cygpath -w "${COVERAGE_DIR}/coverage.xml")" \
            -- "$(cygpath -w "${THREAD_TEST_EXE}")" || { echo "Coverage generation failed for EventLoopThreadSafetyTests."; exit 1; }

        echo "Coverage report generated at: ${COVERAGE_DIR}/html/index.html"
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
