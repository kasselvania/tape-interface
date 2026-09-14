/* Small public-API helpers for the two routing probes. No firmware internals. */
#ifndef PROBE_SUPPORT_H
#define PROBE_SUPPORT_H
#include "tapp_api.h"

/* Single aligned words cross the UI/audio boundary. No locks, allocations or
 * firmware UI calls in process(); native and ARM tests use the same code. */
_Static_assert(__atomic_always_lock_free(sizeof(uint32_t), 0), "32-bit atomics required");
static uint32_t probe_load(const uint32_t* p) { return __atomic_load_n(p, __ATOMIC_RELAXED); }
static void probe_store(uint32_t* p, uint32_t v) { __atomic_store_n(p, v, __ATOMIC_RELAXED); }
static unsigned probe_peak_units(float x) {
    union { float f; uint32_t u; } v = {.f = x};
    v.u &= 0x7fffffffu;
    /* Numeric display: 1000 = 100.0% FS; 65535 also flags non-finite input.
     * Bit check survives the SDK's fast-math compilation. No sample limiting. */
    if (v.u >= 0x428311ecu) return 65535; /* ~65.535, also NaN/Inf */
    return (unsigned)(v.f * 1000.f + 0.5f);
}
static uint32_t probe_peaks(const float* in, uint32_t n) {
    unsigned left = 0, right = 0;
    if (in) for (uint32_t i = 0; i < n; i++) {
        unsigned p = probe_peak_units(in[i]);
        if (i & 1u) { if (p > right) right = p; }
        else if (p > left) left = p;
    }
    return left | (right << 16);
}
static void probe_draw_peaks(gfx_t* gfx, uint32_t peaks, unsigned y) {
    unsigned l = peaks & 65535u, r = peaks >> 16;
    gfx_draw_strf(gfx, 10, y, "Input peak L %u.%u R %u.%u %%FS",
                  l / 10, l % 10, r / 10, r % 10);
}
static void probe_draw_state(gfx_t* gfx, unsigned y) {
    uint8_t source = os_audio_get_input();
    gfx_draw_strf(gfx, 10, y, "Monitor Flag: %s   %s",
                  os_audio_get_monitor() ? "ON" : "OFF",
                  source == 0 ? "LINE" : source == 1 ? "MIC" : "N/A");
}
#endif
