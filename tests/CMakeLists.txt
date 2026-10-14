cmake_policy(SET CMP0135 NEW)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

add_executable(EventLoopTests EventLoopBasicTest.cpp)

target_link_libraries(EventLoopTests
    PRIVATE
    gtest
    gtest_main
    EventLoop
)

# The event loop is a process-wide singleton that cannot run again after Halt(), so this test
# gets its own executable.
add_executable(EventLoopThreadSafetyTests EventLoopThreadSafetyTest.cpp)

target_link_libraries(EventLoopThreadSafetyTests
    PRIVATE
    gtest
    gtest_main
    EventLoop
)

if(TEST_COVERAGE AND NOT MSVC)
    foreach(test_target EventLoopTests EventLoopThreadSafetyTests)
        target_compile_options(${test_target} PRIVATE -coverage)
        target_link_options(${test_target} PRIVATE -coverage)
    endforeach()
endif()

add_test(NAME EventLoopBasicTest COMMAND EventLoopTests)
add_test(NAME EventLoopThreadSafetyTest COMMAND EventLoopThreadSafetyTests)
# A lost wakeup shows up as a hang.
set_tests_properties(EventLoopBasicTest EventLoopThreadSafetyTest PROPERTIES TIMEOUT 120)

if(WIN32)
    set(_test_path "$<TARGET_FILE_DIR:EventLoop>;$ENV{PATH}")
    if(MINGW)
        get_filename_component(_compiler_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        string(APPEND _test_path ";${_compiler_bin}")
    endif()
    string(REPLACE ";" "\\;" _test_path "${_test_path}")
    set_tests_properties(EventLoopBasicTest EventLoopThreadSafetyTest PROPERTIES
        ENVIRONMENT "PATH=${_test_path}"
    )
endif()
