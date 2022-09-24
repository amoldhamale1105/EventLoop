cmake_minimum_required(VERSION 3.0.0)

project(BaseProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(INCLUDES
    include/App.hpp)
set(SOURCES
    src/App.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME} 
    ${INCLUDES}
    ${SOURCES}
    main.cpp)

# add_library(${PROJECT_NAME} SHARED
#     ${INCLUDES}
#     ${SOURCES})