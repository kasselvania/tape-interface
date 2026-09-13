/* Adapted from tape_sdk/examples/simple_app.c.
 * Copyright (c) 2026 Bedtime LLC. MIT license; see the repository LICENSE.
 */
#include "tapp_api.h"

static bool interface_init(os_app_t* app, va_list args) {
    (void)app;
    (void)args;
    static const tapp_hint_pair_t hints[5] = {
        {0, 0}, {0, 0}, {0, 0}, {0, "exit"}, {0, 0},
    };
    ui_statusbar_show(true);
    ui_hints_set_labels(hints);
    ui_hints_show(true);
    return true;
}

static bool interface_deinit(os_app_t* app) {
    (void)app;
    return true;
}

static void interface_redraw(gfx_t* gfx, const os_app_t* app) {
    (void)app;
    /* The firmware owns the hint band at rows 0..59. */
    gfx_set_color(gfx, 1);
    gfx_draw_str(gfx, 10, 90, "Tape Interface");
}

static void interface_input(os_app_t* app, uint8_t btn, KeyStateEnum state) {
    (void)app;
    if (btn == 3 && state == KEY_STATE_HOLD) {
        os_app_exit();
    }
}

static os_app_data_t interface_data = {
    .model = NULL,
    .model_size = 0,
    .init = interface_init,
    .deinit = interface_deinit,
};

static os_app_t interface_app = {
    .name = "Tape Interface",
    .type = AppFullscreenType,
    .data = &interface_data,
    .redraw = interface_redraw,
    .on_input = interface_input,
};

os_app_t* tapp_get_descriptor(void) {
    return &interface_app;
}
