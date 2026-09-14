/* Firmware-control patterns from tape_sdk/examples/recorder.c; UI/lifecycle
 * adapted from tape_sdk/examples/simple_app.c.
 * Copyright (c) 2026 Bedtime LLC. MIT license; see the repository LICENSE.
 */
#include "tapp_api.h"

/* Observed firmware values only. Never cache requested monitor intent. */
typedef struct {
    uint8_t input;
    bool monitor;
    int gain_percent;  /* -1 unavailable; -2 outside the SDK's 0..1 contract. */
    bool dirty;
} interface_model_t;

static params_t* interface_input_volume(void) {
    mixer_t* mix = mixer_get();
    return mix ? mixer_get_input_vol(mix) : NULL;
}

static int interface_gain_percent(params_t* volume) {
    if (!volume) return -1;
    union { float value; uint32_t bits; } fraction = {
        .value = param_val_percent(volume),
    };
    /* The pinned SDK promises 0..1. Check IEEE-754 bits because its build uses
     * -ffast-math: ordinary float checks can assume NaN/infinity never occur.
     * Positive 0..1 and negative zero are the only accepted representations.
     * Never guess a different firmware scale or clamp it into a valid label. */
    if (fraction.bits > 0x3f800000u && fraction.bits != 0x80000000u) return -2;
    return (int)(fraction.value * 100.f);
}

static void interface_refresh(os_app_t* app) {
    interface_model_t* model = os_app_get_model(app);
    const uint8_t input = os_audio_get_input();
    const bool monitor = os_audio_get_monitor();
    params_t* volume = interface_input_volume();
    const int gain = interface_gain_percent(volume);

    if (model->input != input || model->monitor != monitor ||
        model->gain_percent != gain) {
        model->dirty = true;
    }
    model->input = input;
    model->monitor = monitor;
    model->gain_percent = gain;
}

static bool interface_init(os_app_t* app, va_list args) {
    (void)args;
    interface_model_t* model = os_app_get_model(app);
    if (!model) return false;
    static const tapp_hint_pair_t hints[5] = {
        {"mon", 0}, {0, "src"}, {0, 0}, {0, "exit"}, {0, 0},
    };
    ui_statusbar_show(true);
    ui_hints_set_labels(hints);
    ui_hints_show(true);
    interface_refresh(app);
    model->dirty = true;
    return true;
}

static bool interface_deinit(os_app_t* app) {
    (void)app;
    return true;
}

static void interface_redraw(gfx_t* gfx, const os_app_t* app) {
    const interface_model_t* model = os_app_get_model(app);
    /* The firmware owns the hint band at rows 0..59. */
    gfx_set_color(gfx, 1);
    gfx_draw_str(gfx, 10, 82, "Tape Interface");
    gfx_draw_strf(gfx, 10, 110, "INPUT: %s",
                  model->input == 0 ? "LINE" : model->input == 1 ? "MIC" : "N/A");
    if (model->gain_percent == -2) {
        gfx_draw_str(gfx, 10, 134, "INPUT GAIN: N/A (range)");
    } else if (model->gain_percent < 0) {
        gfx_draw_str(gfx, 10, 134, "INPUT GAIN: N/A");
    } else {
        gfx_draw_strf(gfx, 10, 134, "INPUT GAIN: %d%%", model->gain_percent);
    }
    gfx_draw_strf(gfx, 10, 158, "MONITOR: %s", model->monitor ? "ON" : "OFF");
    gfx_draw_str(gfx, 10, 184, "STOCK ROUTE / NO DSP");
    gfx_draw_str(gfx, 10, 212, "ENC: input gain (saved)");
}

static bool interface_tick(os_app_t* app) {
    interface_model_t* model = os_app_get_model(app);
    /* Source, effective monitoring and gain can change outside this app. */
    interface_refresh(app);
    const bool redraw = model->dirty;
    model->dirty = false;
    return redraw;
}

static void interface_input(os_app_t* app, uint8_t btn, KeyStateEnum state) {
    if (btn == 5) {
        /* Like recorder.c: rotation arrives as RELEASED, not a button press.
         * Always drain the accumulator, even when the parameter is absent. */
        const int32_t delta = os_controls_encoder_get_delta();
        params_t* volume = interface_input_volume();
        if (volume && delta) {
            param_write_delta_val(volume, delta);
            /* The firmware callback applies monitor/USB gain and saves it. */
            param_update_fast(volume, NULL);
        }
    } else if (btn == 0 && state == KEY_STATE_PRESSED) {
        os_audio_set_monitor(!os_audio_get_monitor());
    } else if (btn == 1 && state == KEY_STATE_HOLD) {
        /* Persistent device setting: deliberate hold only. */
        os_audio_switch_input(true);
    } else if (btn == 3 && state == KEY_STATE_HOLD) {
        os_app_exit();
        return;
    } else {
        return;
    }
    /* Read back effective state, including refused monitor-enable requests. */
    interface_refresh(app);
}

static os_app_data_t interface_data = {
    .model = NULL,
    .model_size = sizeof(interface_model_t),
    .init = interface_init,
    .deinit = interface_deinit,
};

static os_app_t interface_app = {
    .name = "Tape Interface",
    .type = AppFullscreenType,
    .data = &interface_data,
    .redraw = interface_redraw,
    .tick = interface_tick,
    .on_input = interface_input,
};

os_app_t* tapp_get_descriptor(void) {
    return &interface_app;
}
