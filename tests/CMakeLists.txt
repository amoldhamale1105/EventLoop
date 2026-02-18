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

if(NOT MSVC)
    target_compile_options(EventLoopTests PRIVATE -coverage)
    target_link_options(EventLoopTests PRIVATE -coverage)
endif()

add_test(NAME EventLoopBasicTest COMMAND EventLoopTests)

if(WIN32)
    set_tests_properties(EventLoopBasicTest PROPERTIES 
        ENVIRONMENT "PATH=$<TARGET_FILE_DIR:EventLoop>;$ENV{PATH}"
    )
endif()