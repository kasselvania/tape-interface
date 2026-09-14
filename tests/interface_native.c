/* Contract checks against the actual TAPP callbacks and pinned SDK types.
 * These stubs model firmware control behavior; they do not prove hardware audio.
 */
/* Read the SDK's libc declarations before macOS adds fortified libc macros. */
#include "../tapp/interface/interface.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct mixer_s { int unused; };
struct gfx_t { int unused; };
static struct mixer_s firmware_mixer;
static struct gfx_t display;
static interface_model_t model;
static bool monitor, refuse_monitor, have_mixer, have_volume;
static uint8_t source;
static int32_t encoder_delta, written_delta;
static unsigned monitor_sets, source_switches, exits, writes, updates, callbacks;
static unsigned volume_reads, encoder_reads, percent_reads;
static bool override_percent;
static float percent_result;
static char parameter_calls[32];
static char rendered[1024];
static os_app_t* app;

static void volume_callback(params_t* p, void* ctx);
static params_t volume = {
    .min = 10.f, .max = 30.f, .fine = 0.5f, .refresh = true,
    .callback = volume_callback,
};

void* os_app_get_model(const os_app_t* a) { return a->data->model; }
void os_app_exit(void) { exits++; }
uint8_t os_audio_get_input(void) { return source; }
bool os_audio_get_monitor(void) { return monitor; }
bool os_audio_set_monitor(bool requested) {
    monitor_sets++;
    monitor = requested && !refuse_monitor;
    return monitor;
}
uint8_t os_audio_switch_input(bool line_mic_only) {
    assert(line_mic_only);
    source_switches++;
    source = source == 0 ? 1 : 0;
    /* Firmware can also change effective monitoring on a source change. */
    if (source == 1 && refuse_monitor) monitor = false;
    return source;
}
mixer_t* mixer_get(void) { return have_mixer ? &firmware_mixer : NULL; }
params_t* mixer_get_input_vol(mixer_t* mix) {
    assert(mix == &firmware_mixer);
    volume_reads++;
    return have_volume ? &volume : NULL;
}
int32_t os_controls_encoder_get_delta(void) {
    encoder_reads++;
    int32_t result = encoder_delta;
    encoder_delta = 0;
    return result;
}
static void volume_callback(params_t* p, void* ctx) {
    assert(p == &volume && ctx == NULL);
    callbacks++;
}
void param_write_delta_val(params_t* p, int32_t delta) {
    assert(p == &volume);
    strcat(parameter_calls, "W");
    writes++;
    written_delta = delta;
    p->target += p->fine * (float)delta;
    if (p->target < p->min) p->target = p->min;
    if (p->target > p->max) p->target = p->max;
}
bool param_update_fast(params_t* p, void* ctx) {
    assert(p == &volume && ctx == NULL);
    assert(writes == updates + 1); /* Write, then update, exactly once per event. */
    strcat(parameter_calls, "U");
    updates++;
    bool changed = p->val != p->target;
    p->val = p->target;
    if (changed) p->callback(p, ctx);
    return changed;
}
float param_val_percent(const params_t* p) {
    assert(p == &volume);
    percent_reads++;
    if (override_percent) return percent_result;
    return (p->val - p->min) / (p->max - p->min);
}
void ui_statusbar_show(bool on) { assert(on); }
void ui_hints_show(bool on) { assert(on); }
void ui_hints_set_labels(const tapp_hint_pair_t* hints) {
    /* Press/hold labels must match the actual gestures. */
    const tapp_hint_pair_t expected[5] = {
        {"mon", 0}, {0, "src"}, {0, 0}, {0, "exit"}, {0, 0},
    };
    for (unsigned i = 0; i < 5; i++) {
        assert(hints[i].press == expected[i].press ||
               (hints[i].press && expected[i].press &&
                strcmp(hints[i].press, expected[i].press) == 0));
        assert(hints[i].hold == expected[i].hold ||
               (hints[i].hold && expected[i].hold &&
                strcmp(hints[i].hold, expected[i].hold) == 0));
    }
}
void gfx_set_color(gfx_t* gfx, uint8_t color) {
    assert(gfx == &display && color == 1);
}
gfx_uint_t gfx_draw_str(gfx_t* gfx, gfx_uint_t x, gfx_uint_t y, const char* text) {
    assert(gfx == &display && x == 10 && y >= 60 && y < 240);
    assert(strlen(rendered) + strlen(text) + 2 < sizeof(rendered));
    strcat(rendered, text);
    strcat(rendered, "\n");
    return 0;
}
gfx_uint_t gfx_draw_strf(gfx_t* gfx, gfx_uint_t x, gfx_uint_t y, const char* fmt, ...) {
    char text[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);
    return gfx_draw_str(gfx, x, y, text);
}

static bool initialize(os_app_t* a, ...) {
    va_list args;
    va_start(args, a);
    bool result = a->data->init(a, args);
    va_end(args);
    return result;
}
static void reset(void) {
    memset(&model, 0, sizeof(model));
    monitor = false; refuse_monitor = false; source = 0;
    have_mixer = true; have_volume = true;
    encoder_delta = written_delta = 0;
    monitor_sets = source_switches = exits = writes = updates = callbacks = 0;
    volume_reads = encoder_reads = percent_reads = 0;
    override_percent = false;
    volume.min = 10.f; volume.max = 30.f;
    parameter_calls[0] = rendered[0] = '\0';
    volume.val = volume.target = 20.f;
    app = tapp_get_descriptor();
    app->data->model = &model;
    assert(app->type == AppFullscreenType);
    assert(app->engine_cb == NULL);
    assert(app->data->model_size == sizeof(model));
}
static void render_has(const char* text) {
    rendered[0] = '\0';
    app->redraw(&display, app);
    assert(strstr(rendered, text));
    assert(strstr(rendered, "Tape Interface\n"));
    assert(strstr(rendered, "STOCK ROUTE / NO DSP\n"));
}

static void test_lifecycle(void) {
    reset();
    source = 1; monitor = true;
    volume.target = 25.f; /* Init must not even advance an existing target. */
    assert(initialize(app));
    render_has("INPUT: MIC\n");
    render_has("MONITOR: ON\n");
    render_has("INPUT GAIN: 50.00%\n");
    assert(monitor_sets == 0 && source_switches == 0 && writes == 0 && updates == 0);
    assert(volume.val == 20.f && volume.target == 25.f);
    assert(app->data->deinit(app));
    assert(monitor && source == 1 && volume.val == 20.f);
    assert(monitor_sets == 0 && source_switches == 0 && writes == 0 && updates == 0);
    app->data->model = NULL;
    assert(!initialize(app));
    puts("PASS lifecycle: read-only init/deinit, null model, no audio callbacks");
}

static void test_buttons(void) {
    for (uint8_t button = 0; button < 5; button++) {
        for (KeyStateEnum state = KEY_STATE_RELEASED; state < KEY_STATE_TOTAL; state++) {
            reset();
            assert(initialize(app));
            app->on_input(app, button, state);
            assert(monitor_sets == (unsigned)(button == 0 && state == KEY_STATE_PRESSED));
            assert(source_switches == (unsigned)(button == 1 && state == KEY_STATE_HOLD));
            assert(exits == (unsigned)(button == 3 && state == KEY_STATE_HOLD));
            assert(writes == 0 && updates == 0 && encoder_reads == 0);
            render_has(monitor ? "MONITOR: ON\n" : "MONITOR: OFF\n");
            render_has(source ? "INPUT: MIC\n" : "INPUT: LINE\n");
        }
    }
    reset(); assert(initialize(app));
    app->on_input(app, 0, KEY_STATE_PRESSED);
    app->on_input(app, 0, KEY_STATE_HOLD);
    app->on_input(app, 0, KEY_STATE_RELEASED);
    assert(monitor_sets == 1 && monitor);
    app->on_input(app, 0, KEY_STATE_PRESSED);
    assert(monitor_sets == 2 && !monitor);
    app->on_input(app, 1, KEY_STATE_PRESSED);
    app->on_input(app, 1, KEY_STATE_HOLD);
    app->on_input(app, 1, KEY_STATE_RELEASED);
    assert(source_switches == 1 && source == 1);
    app->on_input(app, 1, KEY_STATE_HOLD);
    assert(source_switches == 2 && source == 0);
    puts("PASS buttons: all 15 button/state pairs and complete gestures");
}

static void test_effective_state(void) {
    reset(); source = 1; refuse_monitor = true;
    assert(initialize(app));
    app->on_input(app, 0, KEY_STATE_PRESSED);
    assert(monitor_sets == 1 && !monitor);
    render_has("MONITOR: OFF\n");
    app->on_input(app, 1, KEY_STATE_HOLD);
    render_has("INPUT: LINE\n");
    monitor = true;
    app->on_input(app, 1, KEY_STATE_HOLD);
    render_has("INPUT: MIC\n");
    render_has("MONITOR: OFF\n");
    puts("PASS effective state: refused enable and source-triggered monitor change");
}

static void test_external_refresh(void) {
    reset(); assert(initialize(app));
    assert(app->tick(app));
    assert(!app->tick(app));
    monitor = true;
    assert(app->tick(app)); render_has("MONITOR: ON\n");
    assert(!app->tick(app));
    source = 1;
    assert(app->tick(app)); render_has("INPUT: MIC\n");
    volume.val = 25.f;
    assert(app->tick(app)); render_has("INPUT GAIN: 75.00%\n");
    have_volume = false;
    assert(app->tick(app)); render_has("INPUT GAIN: N/A\n");
    have_volume = true;
    assert(app->tick(app)); render_has("INPUT GAIN: 75.00%\n");
    assert(monitor_sets == 0 && source_switches == 0 && writes == 0 && updates == 0);
    puts("PASS refresh: external monitor/source/gain and availability changes");
}

static void test_encoder(void) {
    for (KeyStateEnum state = KEY_STATE_RELEASED; state < KEY_STATE_TOTAL; state++) {
        reset(); assert(initialize(app));
        encoder_delta = 4;
        app->on_input(app, 5, state);
        assert(writes == 1 && updates == 1 && callbacks == 1 && written_delta == 4);
        assert(strcmp(parameter_calls, "WU") == 0 && encoder_reads == 1);
        assert(volume.val == 22.f && percent_reads >= 2);
        render_has("INPUT GAIN: 60.00%\n");
        encoder_delta = -8;
        app->on_input(app, 5, state);
        assert(writes == 2 && updates == 2 && callbacks == 2 && written_delta == -8);
        assert(strcmp(parameter_calls, "WUWU") == 0);
        render_has("INPUT GAIN: 40.00%\n");
        app->on_input(app, 5, state); /* Zero movement must not write or update. */
        assert(writes == 2 && updates == 2 && encoder_reads == 3);
    }
    encoder_delta = 1000;
    app->on_input(app, 5, KEY_STATE_RELEASED);
    render_has("INPUT GAIN: 100.00%\n");
    encoder_delta = -1000;
    app->on_input(app, 5, KEY_STATE_RELEASED);
    render_has("INPUT GAIN: 0.00%\n");
    puts("PASS encoder: signed/zero deltas, API order, callback, normalized percentage");
}

static void test_unavailable_volume(void) {
    for (unsigned missing_mixer = 0; missing_mixer < 2; missing_mixer++) {
        reset();
        have_mixer = !missing_mixer;
        have_volume = false;
        assert(initialize(app));
        render_has("INPUT GAIN: N/A\n");
        encoder_delta = 99;
        app->on_input(app, 5, KEY_STATE_RELEASED);
        assert(encoder_delta == 0 && encoder_reads == 1);
        assert(writes == 0 && updates == 0 && percent_reads == 0);
        if (missing_mixer) assert(volume_reads == 0);
        app->on_input(app, 0, KEY_STATE_PRESSED);
        render_has("MONITOR: ON\n");
        app->on_input(app, 1, KEY_STATE_HOLD);
        render_has("INPUT: MIC\n");
        have_mixer = have_volume = true;
        app->on_input(app, 5, KEY_STATE_RELEASED);
        assert(writes == 0 && updates == 0); /* No stale movement after recovery. */
        render_has("INPUT GAIN: 50.00%\n");
    }
    puts("PASS null mixer/volume: N/A, safe controls, drained encoder, recovery");
}

static void test_exit_relaunch(void) {
    reset(); assert(initialize(app));
    app->on_input(app, 0, KEY_STATE_PRESSED);
    app->on_input(app, 1, KEY_STATE_HOLD);
    encoder_delta = 10;
    app->on_input(app, 5, KEY_STATE_RELEASED);
    app->on_input(app, 3, KEY_STATE_HOLD);
    assert(exits == 1);
    assert(app->data->deinit(app));
    memset(&model, 0, sizeof(model));
    assert(initialize(app));
    render_has("INPUT: MIC\n"); render_has("MONITOR: ON\n");
    render_has("INPUT GAIN: 75.00%\n");
    assert(monitor_sets == 1 && source_switches == 1 && writes == 1 && updates == 1);
    puts("PASS same-boot exit/relaunch: no restoration or initialization writes");
}

static void test_invalid_gain_readback(void) {
    reset(); assert(initialize(app));
    assert(app->tick(app));
    override_percent = true;
    const union { float value; uint32_t bits; } invalid[] = {
        {.value = 490.f}, {.value = 1.01f}, {.value = -0.01f},
        {.bits = 0x7f800000u}, {.bits = 0xff800000u}, {.bits = 0x7fc00000u},
    };
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        percent_result = 0.5f;
        app->tick(app);
        render_has("INPUT GAIN: 50.00%\n");
        percent_result = invalid[i].value;
        assert(app->tick(app));
        render_has("INPUT GAIN: N/A (range)\n");
        assert(model.gain_bits[0] == invalid[i].bits);
        render_has("MIN 41200000  MAX 41F00000\n");
        assert(!app->tick(app));
    }
    percent_result = 49.f;
    assert(app->tick(app));
    volume.val = volume.target = 21.f;
    assert(app->tick(app));
    render_has("API 42440000  VAL 41A80000\n");
    assert(!app->tick(app));
    percent_result = 0.55f;
    assert(app->tick(app)); render_has("INPUT GAIN: 55.00%\n");
    assert(strstr(rendered, "API ") == NULL);
    render_has("ENC: input gain (saved)\n");
    const float invalid_bounds[][3] = {
        {20.f, 10.f, 10.f}, {20.f, 30.f, 10.f}, {31.f, 10.f, 30.f},
        {9.f, 10.f, 30.f},
    };
    for (unsigned i = 0; i < sizeof(invalid_bounds)/sizeof(invalid_bounds[0]); i++) {
        volume.val = invalid_bounds[i][0];
        volume.min = invalid_bounds[i][1]; volume.max = invalid_bounds[i][2];
        app->tick(app); render_has("INPUT GAIN: N/A (range)\n");
    }
    for (unsigned i = 3; i < 6; i++) {
        for (unsigned field = 0; field < 3; field++) {
            volume.val = 20.f; volume.min = 10.f; volume.max = 30.f;
            float* fields[] = {&volume.val, &volume.min, &volume.max};
            *fields[field] = invalid[i].value;
            app->tick(app); render_has("INPUT GAIN: N/A (range)\n");
        }
    }
    assert(monitor_sets == 0 && source_switches == 0 && writes == 0 && updates == 0);
    puts("PASS invalid gain: inconsistent API, invalid bounds, non-finite fields, diagnostics, recovery");
}

static void test_gain_scales(void) {
    reset(); assert(initialize(app)); app->tick(app);
    override_percent = true;
    volume.min = 0.f; volume.max = 2.f;
    /* Exact paired operator samples; click count was uncertain. */
    const union { float value; uint32_t bits; } observed[] = {
        {.bits = 0x3dc8b43au}, {.bits = 0x3dcac084u},
    };
    const float api[] = {49.f, 49.5f};
    const char* labels[] = {"INPUT GAIN: 4.90%\n", "INPUT GAIN: 4.95%\n"};
    for (unsigned i = 0; i < 2; i++) {
        volume.val = volume.target = observed[i].value;
        percent_result = api[i];
        assert(app->tick(app)); render_has(labels[i]);
        assert(strstr(rendered, "API ") == NULL);
        assert(!app->tick(app));
    }
    /* Synthetic contract checks across both scales, including near-zero
     * thousandths <= 1, where selecting scale by magnitude would fail. */
    const float fractions[] = {0.f, 0.0005f, 0.001f, 0.25f, 0.5f, 1.f};
    const char* expected[] = {"0.00", "0.05", "0.10", "25.00", "50.00", "100.00"};
    for (unsigned offset = 0; offset < 2; offset++) {
        volume.min = offset ? 10.f : 0.f;
        volume.max = offset ? 30.f : 2.f;
        for (unsigned scale = 0; scale < 2; scale++) {
            for (unsigned i = 0; i < sizeof(fractions)/sizeof(fractions[0]); i++) {
                volume.val = volume.min + fractions[i] * (volume.max - volume.min);
                percent_result = fractions[i] * (scale ? 1000.f : 1.f);
                app->tick(app);
                char label[64];
                snprintf(label, sizeof(label), "INPUT GAIN: %s%%\n", expected[i]);
                render_has(label);
            }
        }
    }
    volume.val = volume.min;
    percent_result = -0.f;
    app->tick(app); render_has("INPUT GAIN: 0.00%\n");
    assert(monitor_sets == 0 && source_switches == 0 && writes == 0 && updates == 0);
    puts("PASS gain scales: exact hardware samples, normalized/thousandths, small values, endpoints, offset ranges");
}

int main(void) {
    test_lifecycle();
    test_buttons();
    test_effective_state();
    test_external_refresh();
    test_encoder();
    test_unavailable_volume();
    test_exit_relaunch();
    test_invalid_gain_readback();
    test_gain_scales();
    puts("PASS: 9 native firmware-stub groups (physical audio NOT TESTED)");
    return 0;
}
