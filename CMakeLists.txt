project(bonk)
cmake_minimum_required(VERSION 3.5.0)

include(FindPkgConfig)
pkg_check_modules(
    XCB_LIBS
        xcb
        xcb-ewmh
        xcb-icccm
    REQUIRED
)

if(WITH_DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
endif()
if(WITH_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
endif(WITH_COVERAGE)

set(
    BONK_COMMANDS
        src/commands/b_activate.c
        src/commands/b_close.c
        src/commands/b_decoration.c
        src/commands/b_focus.c
        src/commands/b_get_active.c
        src/commands/b_get_focus.c
        src/commands/b_get_title.c
        src/commands/b_help.c
        src/commands/b_lower.c
        src/commands/b_map.c
        src/commands/b_move_resize.c
        src/commands/b_pick.c
        src/commands/b_prop_adjust.c
        src/commands/b_prop_delete.c
        src/commands/b_raise.c
        src/commands/b_reject.c
        src/commands/b_select.c
        src/commands/b_sleep.c
        src/commands/b_state.c
        src/commands/b_unmap.c
)

add_executable(
    bonk
        src/bonk_buffers.c
        src/bonk.c
        src/bonk_command_api.c
        src/bonk_select_api.c
        src/bonk_select_impl.c
        src/xpick.c
        ${BONK_COMMANDS}
)

target_include_directories(
    bonk
    PRIVATE
        src/
)

target_link_libraries(
    bonk
        xcb
        xcb-ewmh
        xcb-icccm
)

install(
    TARGETS
        bonk
    DESTINATION
        bin
)
