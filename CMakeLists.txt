cmake_minimum_required(VERSION 3.20)

project(Lab1_GroupProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    Lab1App
    main.cpp
    shared_types.h
    student_a.cpp
    student_b.cpp
)

if(MSVC)
    target_compile_options(Lab1App PRIVATE /W4 /utf-8)
else()
    target_compile_options(Lab1App PRIVATE -Wall -Wextra -Wpedantic)
endif()