cmake_minimum_required(VERSION 3.0.0)

project(EventLoop VERSION 1.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_library(${PROJECT_NAME} SHARED
    ${INCLUDES}
    ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})