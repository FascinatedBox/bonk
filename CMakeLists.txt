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

find_program(HAVE_RONN ronn)

if(WITH_DOCS AND NOT HAVE_RONN)
    message(FATAL_ERROR "Cannot build documentation (HAVE_DOCS) without ronn.")
endif()

if(WITH_DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0")
endif()
if(WITH_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
endif(WITH_COVERAGE)

set(
    BONK_DOCS
        doc/bonk.1
        doc/bonk-activate.1
        doc/bonk-close.1
        doc/bonk-decoration.1
        doc/bonk-focus.1
        doc/bonk-get-active.1
        doc/bonk-get-focus.1
        doc/bonk-get-title.1
        doc/bonk-lower.1
        doc/bonk-map.1
        doc/bonk-move-resize.1
        doc/bonk-opacity.1
        doc/bonk-pick.1
        doc/bonk-raise.1
        doc/bonk-reject.1
        doc/bonk-select.1
        doc/bonk-set-window.1
        doc/bonk-sleep.1
        doc/bonk-state.1
        doc/bonk-terminate.1
        doc/bonk-unmap.1
)

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
        src/commands/b_opacity.c
        src/commands/b_pick.c
        src/commands/b_raise.c
        src/commands/b_reject.c
        src/commands/b_select.c
        src/commands/b_set_window.c
        src/commands/b_sleep.c
        src/commands/b_state.c
        src/commands/b_terminate.c
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

if(WITH_DOCS)
    foreach(DOC ${BONK_DOCS})
        add_custom_command(
            OUTPUT  ${PROJECT_BINARY_DIR}/${DOC}
            DEPENDS ${PROJECT_SOURCE_DIR}/${DOC}.ronn
            COMMAND ronn --roff ${PROJECT_SOURCE_DIR}/${DOC}.ronn
        )

        list(APPEND DOC_RESULTS ${PROJECT_BINARY_DIR}/${DOC})
    endforeach()

    add_custom_target(
        docs
            ALL
        DEPENDS
            ${DOC_RESULTS}
    )

    install(
        FILES
            ${BONK_DOCS}
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/man/man1
    )
else()
    message(STATUS "")
    message(STATUS "Skipping man pages (HAVE_DOCS) not set.")
    message(STATUS "Rebuild with -DWITH_DOCS=true and install ronn to build man pages.")
    message(STATUS "")
endif()
