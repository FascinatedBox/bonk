project(boxdotool)
cmake_minimum_required(VERSION 3.0.0)

include(FindPkgConfig)

if(WITH_DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
endif()

if(WITH_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
endif(WITH_COVERAGE)

pkg_check_modules(
    XCB_LIBS
        xcb
        xcb-ewmh
        xcb-icccm
    REQUIRED
)

set(BONK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
