include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(EventLoopTests EventLoopBasicTest.cpp)

target_link_libraries(EventLoopTests
    PRIVATE
    gtest
    gtest_main
    EventLoop
)

target_compile_options(EventLoopTests PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(EventLoopTests PRIVATE -fprofile-arcs -ftest-coverage)

add_test(NAME EventLoopBasicTest COMMAND EventLoopTests)