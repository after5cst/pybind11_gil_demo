cmake_minimum_required(VERSION 3.2)

# Require C++14 or newer.  It is now > 2017
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(
    ENABLE_CPP_WARNINGS
   "Enable more verbose C++ warnings?"
    ON
)
message(STATUS "option ENABLE_CPP_WARNINGS=" ${ENABLE_CPP_WARNINGS})

if("${ENABLE_CPP_WARNINGS}" MATCHES "ON")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(GNU_CXX_FLAGS "-pipe -g")
        set(GNU_CXX_FLAGS "${GNU_CXX_FLAGS} -Wall -Wpedantic -Wunused-value -Wunused -Wcast-qual -Wpointer-arith")
        set(GNU_CXX_FLAGS "${GNU_CXX_FLAGS} -Wextra -Wcast-align -Wdisabled-optimization -Wformat-y2k")
        set(GNU_CXX_FLAGS "${GNU_CXX_FLAGS} -Wno-format-extra-args -Wformat-nonliteral -Wformat=2 -Winit-self")
        set(GNU_CXX_FLAGS "${GNU_CXX_FLAGS} -Winvalid-pch -Wunsafe-loop-optimizations -Wmissing-include-dirs")
        set(GNU_CXX_FLAGS "${GNU_CXX_FLAGS} -Wmissing-braces -Wpacked -Wredundant-decls -Wstack-protector")
        set(GNU_CXX_FLAGS "${GNU_CXX_FLAGS} -Wswitch-enum -Wuninitialized -Weffc++ -Wformat-security")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GNU_CXX_FLAGS} -pthread")

        # check GNU CXX compiler version
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9" OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.9")
            add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
        else()
            message(FATAL_ERROR "a gcc compiler with a version higher than 4.9 is needed.")
        endif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9" OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.9")
    endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
endif("${ENABLE_CPP_WARNINGS}" MATCHES "ON")
