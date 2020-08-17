cmake_minimum_required(VERSION 3.10)

project(blt-nng-demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Werror -Wextra -Wzero-as-null-pointer-constant -Winit-self -Wswitch-default -Wfloat-equal -Wno-unused-parameter -fPIC -pthread -march=native")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

find_package(BearLibTerminal REQUIRED)
include_directories(SYSTEM ${BLT_INCLUDE_DIRS})

find_package(nng REQUIRED)
include_directories(SYSTEM ${nng_INCLUDE_DIRS})

set(PROJECT_SOURCES_DIR ${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE HEADERS ${PROJECT_SOURCES_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCES_DIR}/*.cpp)

include_directories(${PROJECT_SOURCES_DIR})

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${BLT_LIBRARIES})
file(COPY ${BLT_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} ${nng_LIBRARIES})
file(COPY ${nng_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
