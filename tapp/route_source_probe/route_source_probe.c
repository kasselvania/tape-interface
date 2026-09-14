/* WET source probe: identity transform of the TAPP Input Bus when held-enabled.
 * Source mode requires firmware 1.2.0. Operator selects WET in native UI.
 * No public WET setter/readback exists in the pinned API. */
#include "../probe_common/probe_support.h"
TAPP_DECLARE_TYPE(AppTypeSrc);

typedef struct { uint32_t enabled, running, peaks; } source_model_t;
static const engine_callbacks_t source_engine;
static void source_process(engine_t* engine, mixer_t* mix) {
    if (!mix) return;
    float* in = mixer_get_in(mix);
    const uint32_t n = mixer_get_fs(mix);
    source_model_t* m = engine_get_ctx(engine);
    if (m) probe_store(&m->peaks, probe_peaks(in, n));
    const bool pass = m && probe_load(&m->enabled) && probe_load(&m->running);
    if (!in) return;
    /* Volatile stores make whole-buffer identity writes explicit; no out/fx
     * bus access, accumulation, gain or effects. */
    volatile float* dst = in;
    for (uint32_t i = 0; i < n; i++) dst[i] = pass ? in[i] : 0.f;
}
static void source_active(engine_t* engine, bool active) {
    source_model_t* m = engine_get_ctx(engine);
    if (m) {
        probe_store(&m->running, active);
        if (!active) probe_store(&m->enabled, 0);
    }
}
static uint_fast8_t source_is_active(engine_t* engine) {
    source_model_t* m = engine_get_ctx(engine);
    return m && probe_load(&m->running);
}
static const engine_callbacks_t source_engine = {
    .process = source_process, .active = source_active, .is_active = source_is_active,
};
static bool source_init(os_app_t* app, va_list args) {
    (void)args;
    source_model_t* m = os_app_get_model(app);
    if (!m) return false;
    probe_store(&m->enabled, 0); probe_store(&m->running, 0); probe_store(&m->peaks, 0);
    static const tapp_hint_pair_t hints[5] = {{0,"pass"},{"zero",0},{0,0},{0,0},{0,0}};
    ui_hints_set_labels(hints); /* BTN4/BTN5 holds belong to firmware. */
    engine_set_callbacks(app, m); engine_set_active(true);
    return true;
}
static bool source_deinit(os_app_t* app) {
    source_model_t* m = os_app_get_model(app);
    if (m) probe_store(&m->enabled, 0);
    engine_set_active(false); engine_clear_callbacks(&source_engine);
    return true;
}
static void source_input(os_app_t* app, uint8_t btn, KeyStateEnum state) {
    source_model_t* m = os_app_get_model(app);
    if (!m) return;
    if (btn == 0 && state == KEY_STATE_HOLD)
        probe_store(&m->enabled, !probe_load(&m->enabled));
    else if (btn == 1 && state == KEY_STATE_PRESSED) probe_store(&m->enabled, 0);
}
static bool source_tick(os_app_t* app) { (void)app; return true; }
static void source_redraw(gfx_t* gfx, const os_app_t* app) {
    const source_model_t* m = os_app_get_model(app);
    gfx_set_color(gfx, 1);
    gfx_draw_str(gfx, 10, 80, "Routing Source Probe");
    gfx_draw_str(gfx, 10, 104, probe_load(&m->enabled) ? "WET INPUT / PASS UNCHANGED" : "WET INPUT / ZERO (DISARMED)");
    probe_draw_state(gfx, 128);
    probe_draw_peaks(gfx, probe_load(&m->peaks), 152);
    gfx_draw_str(gfx, 10, 176, "Select WET in native UI first");
    gfx_draw_str(gfx, 10, 200, "USB Capture: NOT ESTABLISHED");
    gfx_draw_str(gfx, 10, 228, "B1 hold: PASS / B2: ZERO");
}
static os_app_data_t source_data = {
    .model_size = sizeof(source_model_t), .init = source_init, .deinit = source_deinit,
};
static os_app_t source_app = {
    .name = "Routing Source Probe", .type = AppFullscreenType, .data = &source_data,
    .redraw = source_redraw, .tick = source_tick, .on_input = source_input,
    .engine_cb = &source_engine,
};
os_app_t* tapp_get_descriptor(void) { return &source_app; }
