include_directories(.)

set(
    BONK_COMMANDS
        commands/b_activate.c
        commands/b_close.c
        commands/b_decoration.c
        commands/b_focus.c
        commands/b_get_active.c
        commands/b_get_focus.c
        commands/b_get_title.c
        commands/b_help.c
        commands/b_lower.c
        commands/b_map.c
        commands/b_move_resize.c
        commands/b_pick.c
        commands/b_prop_delete.c
        commands/b_prop_adjust.c
        commands/b_raise.c
        commands/b_reject.c
        commands/b_select.c
        commands/b_state.c
        commands/b_unmap.c
)

add_executable(
    bonk
        bonk.c
        bonk_command_api.c
        bonk_buffers.c
        bonk_select_api.c
        bonk_select_impl.c
        xpick.c
        ${BONK_COMMANDS}
)

target_link_libraries(
    bonk
        xcb
        xcb-ewmh
        xcb-icccm
)

set_target_properties(
    bonk
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY
            ${BONK_BINARY_DIR}
)

install(
    TARGETS
        bonk
    DESTINATION
        bin
)
