/* Firmware-control patterns from tape_sdk/examples/recorder.c; UI/lifecycle
 * adapted from tape_sdk/examples/simple_app.c.
 * Copyright (c) 2026 Bedtime LLC. MIT license; see the repository LICENSE.
 */
#include "tapp_api.h"

/* Observed firmware values only. Never cache requested monitor intent. */
typedef struct {
    uint8_t input;
    bool monitor;
    int gain_hundredths;  /* Percent x100; -1 unavailable; -2 inconsistent readback. */
    bool dirty;
    uint32_t gain_bits[4]; /* Invalid readback: API result, val, min, max. */
} interface_model_t;

static params_t* interface_input_volume(void) {
    mixer_t* mix = mixer_get();
    return mix ? mixer_get_input_vol(mix) : NULL;
}

static uint32_t interface_float_bits(float value) {
    union { float value; uint32_t bits; } sample = {.value = value};
    return sample.bits;
}

/* Validate the SDK fraction and the observed firmware 1.1.4 thousandths
 * against the parameter itself. This avoids a magnitude heuristic, which
 * would confuse small thousandths values with normalized values near zero. */
static bool interface_finite(float value) {
    /* Bit checks survive the SDK's -ffast-math assumptions. */
    return (interface_float_bits(value) & 0x7f800000u) != 0x7f800000u;
}

static int interface_gain_hundredths(params_t* volume, uint32_t bits[4]) {
    for (unsigned i = 0; i < 4; i++) bits[i] = 0;
    if (!volume) return -1;
    const float api = param_val_percent(volume);
    const float value = volume->val, min = volume->min, max = volume->max;
    if (interface_finite(api) && interface_finite(value) &&
        interface_finite(min) && interface_finite(max) && max > min &&
        value >= min && value <= max) {
        const float span = max - min;
        if (interface_finite(span)) {
            const float normalized = (value - min) / span;
            const float candidates[2] = {api, api * 0.001f};
            for (unsigned i = 0; i < 2; i++) {
                const float fraction = candidates[i];
                const float error = fraction - normalized;
                if (fraction >= 0.f && fraction <= 1.f &&
                    error >= -0.000001f && error <= 0.000001f) {
                    /* Percent with two decimal places; no float formatting. */
                    return (int)(fraction * 10000.f + 0.5f);
                }
            }
        }
    }
    /* Preserve exact mismatch evidence; never clamp it to a valid percentage.
     * Reading public fields does not modify the firmware-owned parameter. */
    bits[0] = interface_float_bits(api);
    bits[1] = interface_float_bits(value);
    bits[2] = interface_float_bits(min);
    bits[3] = interface_float_bits(max);
    return -2;
}

static void interface_refresh(os_app_t* app) {
    interface_model_t* model = os_app_get_model(app);
    const uint8_t input = os_audio_get_input();
    const bool monitor = os_audio_get_monitor();
    params_t* volume = interface_input_volume();
    uint32_t gain_bits[4];
    const int gain = interface_gain_hundredths(volume, gain_bits);

    if (model->input != input || model->monitor != monitor ||
        model->gain_hundredths != gain) {
        model->dirty = true;
    }
    model->input = input;
    model->monitor = monitor;
    model->gain_hundredths = gain;
    for (unsigned i = 0; i < 4; i++) {
        if (model->gain_bits[i] != gain_bits[i]) model->dirty = true;
        model->gain_bits[i] = gain_bits[i];
    }
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
    if (model->gain_hundredths == -2) {
        gfx_draw_str(gfx, 10, 134, "INPUT GAIN: N/A (range)");
    } else if (model->gain_hundredths < 0) {
        gfx_draw_str(gfx, 10, 134, "INPUT GAIN: N/A");
    } else {
        gfx_draw_strf(gfx, 10, 134, "INPUT GAIN: %d.%02d%%",
                      model->gain_hundredths / 100, model->gain_hundredths % 100);
    }
    gfx_draw_strf(gfx, 10, 158, "MONITOR: %s", model->monitor ? "ON" : "OFF");
    gfx_draw_str(gfx, 10, 184, "STOCK ROUTE / NO DSP");
    if (model->gain_hundredths == -2) {
        /* Hex preserves exact float bits, without another percentage scale. */
        gfx_draw_strf(gfx, 10, 206, "API %08X  VAL %08X",
                      (unsigned)model->gain_bits[0], (unsigned)model->gain_bits[1]);
        gfx_draw_strf(gfx, 10, 228, "MIN %08X  MAX %08X",
                      (unsigned)model->gain_bits[2], (unsigned)model->gain_bits[3]);
    } else {
        gfx_draw_str(gfx, 10, 212, "ENC: input gain (saved)");
    }
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
