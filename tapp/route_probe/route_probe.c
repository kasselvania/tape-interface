/* Routing reconnaissance, standalone engine role. Firmware routing outside
 * the TAPP Output Bus is deliberately not modified. Pinned SDK API 1.8. */
#include "../probe_common/probe_support.h"

typedef enum { METER_ZERO, TONE_L, TONE_R, COPY_INPUT, ANTI_MONITOR, MODE_COUNT } route_mode_t;
#define ROUTE_ARMED 0x100u
#define ROUTE_MODE(c) ((c) & 7u)
#define ROUTE_TRIM(c) (((c) >> 16) & 255u)
#define ROUTE_DEFAULT (10u << 16) /* 10% anti trim; disabled, capped at 25%. */
#define TONE_AMPLITUDE 0.015848932f /* -36 dBFS peak, assuming float FS=1. */
#define PROBE_RATE 48000u          /* Pinned SDK bus rate, frames/sec. */

typedef struct {
    uint32_t control; /* UI publishes mode/arm/trim atomically as one snapshot. */
    uint32_t peaks;
    uint32_t running;
    uint32_t phase;   /* Audio-thread only; integer cycles modulo 48000. */
    uint32_t previous_control; /* Audio-thread only. */
} route_model_t;
static const engine_callbacks_t route_engine;
static const char* const route_names[] = {
    "METER INPUT / ZERO OUTPUT", "TONE OUT L / 997 Hz", "TONE OUT R / 1499 Hz",
    "COPY INPUT TO OUTPUT", "ANTI-MONITOR / EXPERIMENTAL",
};
static void route_process(engine_t* engine, mixer_t* mix) {
    if (!mix) return;
    const float* in = mixer_get_in(mix);
    float* out = mixer_get_out(mix);
    const uint32_t n = mixer_get_fs(mix); /* Total interleaved samples, not frames. */
    route_model_t* m = engine_get_ctx(engine);
    const uint32_t c = m ? probe_load(&m->control) : 0;
    const unsigned mode = ROUTE_MODE(c);
    const bool enabled = m && probe_load(&m->running) && (c & ROUTE_ARMED);
    if (m) {
        probe_store(&m->peaks, probe_peaks(in, n));
        if (c != m->previous_control) m->phase = 0;
        m->previous_control = c;
    }
    if (!out) return;
    for (uint32_t i = 0; i < n; i += 2) {
        /* Read each frame before writing: also safe if in and out alias. */
        const float l = in ? in[i] : 0.f;
        const float r = in && i + 1 < n ? in[i + 1] : 0.f;
        float ol = 0.f, orr = 0.f;
        if (enabled && (mode == TONE_L || mode == TONE_R)) {
            const float tone = TONE_AMPLITUDE * sinf((float)m->phase * (6.28318530718f / PROBE_RATE));
            if (mode == TONE_L) ol = tone; else orr = tone;
            m->phase = (m->phase + (mode == TONE_L ? 997u : 1499u)) % PROBE_RATE;
        } else if (enabled && mode == COPY_INPUT) {
            ol = l; orr = r;
        } else if (enabled && mode == ANTI_MONITOR) {
            unsigned trim = ROUTE_TRIM(c);
            if (trim > 25) trim = 25;
            const float gain = -(float)trim * 0.01f;
            ol = l * gain; orr = r * gain;
        }
        out[i] = ol;
        if (i + 1 < n) out[i + 1] = orr;
    }
}
static void route_active(engine_t* e, bool active) {
    route_model_t* m = engine_get_ctx(e);
    if (!m) return;
    probe_store(&m->running, active);
    if (!active) __atomic_fetch_and(&m->control, ~ROUTE_ARMED, __ATOMIC_RELAXED);
}
static uint_fast8_t route_is_active(engine_t* e) {
    route_model_t* m = engine_get_ctx(e);
    return m && probe_load(&m->running);
}
static const engine_callbacks_t route_engine = {
    .process = route_process, .active = route_active, .is_active = route_is_active,
};
static bool route_init(os_app_t* app, va_list args) {
    (void)args;
    route_model_t* m = os_app_get_model(app);
    if (!m) return false;
    probe_store(&m->control, ROUTE_DEFAULT);
    probe_store(&m->peaks, 0);
    probe_store(&m->running, 0);
    m->phase = m->previous_control = 0;
    static const tapp_hint_pair_t hints[5] = {
        {"mode", 0}, {0, "arm"}, {"zero", 0}, {0, "exit"}, {0, 0},
    };
    ui_statusbar_show(true); ui_hints_set_labels(hints); ui_hints_show(true);
    engine_set_callbacks(app, m); /* Official recorder install/activate order. */
    engine_set_active(true);     /* Process runs, but writes zero until held arm. */
    return true;
}
static bool route_pause(os_app_t* app) {
    route_model_t* m = os_app_get_model(app);
    if (m) __atomic_fetch_and(&m->control, ~ROUTE_ARMED, __ATOMIC_RELAXED);
    return true;
}
static bool route_deinit(os_app_t* app) {
    route_pause(app);
    engine_set_active(false);
    engine_clear_callbacks(&route_engine);
    return true;
}
static void route_input(os_app_t* app, uint8_t button, KeyStateEnum state) {
    route_model_t* m = os_app_get_model(app);
    if (!m) return;
    uint32_t c = probe_load(&m->control);
    if (button == 0 && state == KEY_STATE_PRESSED) {
        /* Every mode selection disarms; an arm cannot carry into another mode. */
        c = (c & 0x00ff0000u) | ((ROUTE_MODE(c) + 1) % MODE_COUNT);
    } else if (button == 1 && state == KEY_STATE_HOLD) {
        if (ROUTE_MODE(c) != METER_ZERO) c ^= ROUTE_ARMED;
    } else if (button == 2 && state == KEY_STATE_PRESSED) {
        c = (c & 0x00ff0000u) | METER_ZERO;
    } else if (button == 3 && state == KEY_STATE_HOLD) {
        route_pause(app); os_app_exit(); return;
    } else if (button == 5) {
        const int32_t d = os_controls_encoder_get_delta();
        if (ROUTE_MODE(c) == ANTI_MONITOR && !(c & ROUTE_ARMED)) {
            /* Saturate delta before adding, including INT32_MIN/MAX. */
            int trim = (int)ROUTE_TRIM(c) + (d > 25 ? 25 : d < -25 ? -25 : d);
            if (trim < 0) trim = 0;
            if (trim > 25) trim = 25;
            c = (c & 0xffffu) | ((uint32_t)trim << 16);
        }
    } else return;
    probe_store(&m->control, c);
}
static bool route_tick(os_app_t* app) { (void)app; return true; }
static void route_redraw(gfx_t* gfx, const os_app_t* app) {
    const route_model_t* m = os_app_get_model(app);
    const uint32_t c = probe_load(&m->control);
    const unsigned mode = ROUTE_MODE(c);
    const bool armed = (c & ROUTE_ARMED) != 0;
    gfx_set_color(gfx, 1);
    gfx_draw_str(gfx, 10, 80, "Routing Probe");
    gfx_draw_str(gfx, 10, 102, route_names[mode < MODE_COUNT ? mode : 0]);
    probe_draw_state(gfx, 124);
    probe_draw_peaks(gfx, probe_load(&m->peaks), 146);
    if (mode == TONE_L || mode == TONE_R)
        gfx_draw_str(gfx, 10, 168, armed ? "Test level: -36 dBFS peak" : "Test level: ZERO (disarmed)");
    else if (mode == ANTI_MONITOR)
        gfx_draw_strf(gfx, 10, 168, "Test trim: -%u%% / %s", ROUTE_TRIM(c), armed ? "ARMED" : "DISARMED");
    else gfx_draw_str(gfx, 10, 168, armed ? "Output: INPUT x 1.00" : "Output: ZERO");
    if (armed && (mode == TONE_L || mode == TONE_R || mode == ANTI_MONITOR)) {
        gfx_set_font(gfx, gfx_nunito_bold_18);
        gfx_draw_str(gfx, 10, 198, mode == ANTI_MONITOR ? "WARNING: ANTI ARMED" : "WARNING: TONE ARMED");
        gfx_set_font(gfx, gfx_nunito_semibold_14);
    } else gfx_draw_str(gfx, 10, 198, armed ? "COPY ARMED / BTN3: ZERO" : "DISARMED / BTN2 hold: arm");
    gfx_draw_str(gfx, 10, 228, "BTN3: ZERO / ENC: disarmed trim");
}
static os_app_data_t route_data = {
    .model_size = sizeof(route_model_t), .init = route_init, .deinit = route_deinit,
};
static os_app_t route_app = {
    .name = "Routing Probe", .type = AppFullscreenType, .data = &route_data,
    .redraw = route_redraw, .tick = route_tick, .on_input = route_input,
    .on_pause = route_pause, .engine_cb = &route_engine,
};
os_app_t* tapp_get_descriptor(void) { return &route_app; }
