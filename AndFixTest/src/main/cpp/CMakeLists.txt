cmake_minimum_required(VERSION 3.4.1)

find_library(
    log-lib
    log
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fpermissive -DDEBUG -O0 -g ")

include_directories(.)

add_library(
    zjandfix
    SHARED
    andfix.cpp
    dalvik/dalvik_method_replace.cpp
    art/art_method_replace.cpp
    art/art_method_replace_4_4.cpp
    art/art_method_replace_5_0.cpp
    art/art_method_replace_5_1.cpp
    art/art_method_replace_6_0.cpp
    art/art_method_replace_7_0.cpp)

target_link_libraries(
    zjandfix
    ${log-lib}
)

