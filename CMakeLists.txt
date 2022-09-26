cmake_minimum_required(VERSION 3.0.0)

project(EventLoop VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(include)

set(INCLUDES
    include/EventLoop.hpp)
set(SOURCES
    src/Common.cpp
    src/Event.cpp
    src/EventLoop.cpp
    src/EventManager.cpp
    src/EventSender.cpp
    src/EventReceiver.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME} 
    ${INCLUDES}
    ${SOURCES}
    main.cpp)

# add_library(${PROJECT_NAME} SHARED
#     ${INCLUDES}
#     ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})