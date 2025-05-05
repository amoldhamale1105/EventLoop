cmake_minimum_required(VERSION 3.1.0)

project(EventLoop VERSION 2.0.0)

find_package(Threads REQUIRED)

include_directories(include)

# INCLUDES variable defines the headers required by apps using this library
set(INCLUDES
    include/Event.h
    include/EventLoop.h)
set(SOURCES
    src/Event.cpp
    src/EventLoop.cpp
    src/EventManager.cpp
    src/EventSender.cpp
    src/EventReceiver.cpp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_library(${PROJECT_NAME} SHARED
    ${INCLUDES}
    ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME ${PROJECT_NAME}
    CXX_STANDARD_REQUIRED YES
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})