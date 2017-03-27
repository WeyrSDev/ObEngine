cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

project(tgui)

file(GLOB_RECURSE PG_TGUI 
    ../../include/tgui/*.hpp
    ../../include/tgui/*.inl
)
file(GLOB_RECURSE HD_TGUI 
    src/*.cpp
)
include_directories(../../include/)
include_directories(${SFML_INCLUDE_DIR})

ADD_DEFINITIONS(-std=c++14)

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -O3")
endif()

add_library(tgui ${PG_TGUI} ${HD_TGUI})