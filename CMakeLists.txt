cmake_minimum_required(VERSION 3.26)
project(debugging_malloc C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(debugging_malloc
        debugmalloc.c
        debugmalloc.h
        dmhelper.c
        dmhelper.h
        driver.c
        getopt.c
        getopt.h
        tailor.h)
