# Stage C1: firmware audio-control probe

Basis: resolved local and remote `main` at
`ef9aca7cf8817c12f39e5f00597a62f4c5a8d2a1` before editing. SDK remains
`a9cae67124f4209833e3f63c1cd97b11a8f73070`, API 1.8. Initial C1 implementation:
`a0ca8c87f69040b2f370d9449e4740a663dc50d8`. Record the exact app revision/hash
when testing follow-up builds.

## Scope and SDK basis

This is a normal fullscreen TAPP controlling the **stock firmware audio path**.
It does not implement that path. The operator's earlier monitoring and USB-audio
observations are recorded in [the hardware session](HARDWARE_SESSION_2026-09-14.md).

The implementation follows the pinned `tapp_api.h` and `examples/recorder.c`:

- BTN1 PRESS: `os_audio_set_monitor(!os_audio_get_monitor())`.
- BTN2 HOLD: `os_audio_switch_input(true)`; input selection is persistent.
- Encoder ID 5: drain `os_controls_encoder_get_delta()` regardless of event
  state, as the recorder does. Guard `mixer_get()` and then
  `mixer_get_input_vol()`. For nonzero movement and an available parameter, call
  `param_write_delta_val()` then `param_update_fast(param, NULL)`, exactly like
  the recorder's device input-volume branch. The firmware callback applies
  monitor/USB gain and settings persistence. No parameter fields are assigned.
- Gain display: `param_val_percent()` on that real parameter, scaled to integer
  percent across its range; this is not a dB or unity-gain percentage display.
  A follow-up guard shows `N/A (range)` if the API result violates its documented
  0..1 range, including non-finite values. It does not infer a different scale.
- BTN4 HOLD: `os_app_exit()`; no restoration of audio settings.

After every control action, and on every UI tick, the app reads input source,
effective monitor state and current gain again. Its 28-byte model holds observed
display values and a redraw flag, never requested monitor intent. State changes
trigger repaint. Null mixer/volume produces `INPUT GAIN: N/A`; monitor/source
controls remain available. Init reads settings only; deinit changes none.

There is no `engine_cb`, engine registration/activation, audio-buffer accessor,
sample copy, meter, DSP, recording, routing, bypass, or USB implementation.

## Local validation

Run from the project root:

```sh
PATH="$(brew --prefix llvm@18)/bin:$PATH" make test check verify
```

On 2026-09-14, Homebrew LLVM 18.1.8 built `build/interface.tapp`, **4,896 bytes**,
SHA-256 `c46db5c6e624aef2369fe83f12dd1d3106b6a3605c581d6c9724b89889022fb5`.
Both SDK verification passes succeeded with every gate run and **18/18 imports**
resolved. `tests/check_imports.py` checked the finished ELF against the exact C1
control/UI import boundary. There are no engine or buffer-processing imports.

The 12 imports added to the scaffold's six are:

```text
gfx_draw_strf                 os_app_get_model
os_audio_get_monitor         os_audio_set_monitor
os_audio_get_input           os_audio_switch_input
os_controls_encoder_get_delta
mixer_get                    mixer_get_input_vol
param_write_delta_val        param_update_fast
param_val_percent
```

Seven native firmware-stub groups passed: lifecycle without setting writes;
all 15 button/state combinations and gesture sequences; refused monitor enable
and source-triggered effective-monitor change; external state/availability
refresh; signed/zero encoder deltas, API order and callback; null mixer/volume
and recovery without stale encoder movement; exit/relaunch without restoration.
The test includes the actual app source and pinned SDK types. Its parameter
stub models the callback contract, not firmware persistence or analog/USB gain.
Native-only `-U__ARM_FP` selects the header's portable math branch. SDK libc
declarations are included before macOS's fortified libc macros. The ARM build
uses the unmodified SDK flags and header.

## Official emulator

The final binary was loaded in [the official browser emulator](https://tapp.b.edti.me/).
The title, `mon`/`src`/`exit` hints, source, monitor, gain-unavailable state and
`STOCK ROUTE / NO DSP` label were visually checked. Short hints fit the system's
fixed-width cells; longer labels did not render fully in this emulator.

The reproducible lifecycle/UI harness uses the same unmodified official
JavaScript/WASM files through the exports used by its web worker. It runs in
Node (validated with v26.0.0), never registers a TAPP engine, and does not
generate audio. Download the official runtime without modifying the SDK:

```sh
mkdir -p build/emulator
curl -fsSL https://tapp.b.edti.me/emu/tapp_emu.js -o build/emulator/tapp_emu.mjs
curl -fsSL https://tapp.b.edti.me/emu/tapp_emu.wasm -o build/emulator/tapp_emu.wasm
node tests/interface_emulator.mjs
```

The `.mjs` suffix selects Node's module loader; file bytes are unchanged.
Recorded runtime SHA-256 values:

- JS: `e1ebfc45f9053fafcaa699466e29bc0da59e2eceb62e1624df864b22d38c7b4c`.
- WASM: `1a06209fbe86b16acc79271d2787048bfdcd45732fd4a425e46fa27e7fcf6d64`.

Passed: load; monitor press toggles ON/OFF; monitor hold leaves state unchanged;
source press leaves LINE unchanged; source hold changes LINE/MIC and back;
encoder with unavailable gain remains safe; BTN4 press does not exit; BTN4 hold
reports `tapp exited — drop another`; reload succeeds. The harness compares
rendered status rows and saves PPM frames and `results.json` under
`build/emulator/`. Inspect those frames as part of UI validation.

**Emulator gap:** the published runtime logs `unresolved API symbol: mixer_get`,
although that function is exported by the pinned SDK contract and passes the
SDK verifier. Consequently input gain is `N/A`; the emulator cannot validate
real device input-gain adjustment or its callback. It confirms
`no engine_cb in descriptor — audio disabled for this tapp`. That message
describes this emulator, not the device's stock firmware audio behavior.
The harness records this known runtime gap explicitly and fails on a changed
unresolved-symbol set so a newer emulator must be reviewed again.

## Hardware readback discrepancy and guarded follow-up

On firmware **1.1.4 (`e924784c`)**, the operator launched the initial C1 build
and reported LINE, gain **50000%**, monitor ON and the expected stock-route and
encoder labels. Launch passed; gain readback failed. See the exact report in
[the hardware session](HARDWARE_SESSION_2026-09-14.md). The discrepancy conflicts
with the documented percentage expectation, but its API/ABI/display cause is
not established. Do not interpret 50000% as actual gain or replace the scale
based on this single display observation.

The follow-up guard preserves the documented conversion for valid values and
shows `N/A (range)` for invalid values, without setting writes. Its IEEE-754 bit
check remains valid under the SDK's `-ffast-math` build flag. The new regression
failed before the guard and passed afterward. Eight native groups now pass,
including out-of-range, NaN/infinity, endpoints and recovery checks; a separate
`-O2 -ffast-math` native build passed those same groups.

Follow-up artifact: **4,964 bytes**, SHA-256
`4f53f1ac4e91a9a19297664baf710e0fa551a0f9778ad7212b775d751b70222c`.
The same `make test check verify` command passed both SDK verification runs and
the unchanged 18-import boundary. `node tests/interface_emulator.mjs` passed
again with the documented missing-mixer limitation. The follow-up was installed
with verified readback and safe ejection; the operator subsequently reported
`INPUT GAIN: N/A (range)`, confirming the invalid API result on hardware. See
[the installation record](HARDWARE_SESSION_2026-09-14.md). It guards an invalid display; it does not resolve
the underlying gain-readback discrepancy or prove physical audio control.

The next diagnostic build adds exact hexadecimal float bits for the API return
and the parameter's `val`, `min`, and `max`, read through the pinned layout.
These two diagnostic rows replace the encoder hint only when gain is invalid.
It preserves the existing control gestures and performs no diagnostic setting
writes or substitute gain calculation. Changes to the diagnostic values trigger
redraw even if gain remains invalid.

Diagnostic artifact: **5,196 bytes**, SHA-256
`6e6a4279443c8595f5d61a81e2eb488f3c82d0f1537f86bd764e223ac214ba6a`.
The same build/verification command, eight native groups (also with optimized
fast-math), and official emulator lifecycle harness pass. Native checks verify
exact diagnostic values, changes while invalid, recovery and no setting writes.
A separate synthetic display fixture rendered readable diagnostic rows in the
official emulator. This verifies layout only; its values are not firmware
measurements. The actual diagnostic binary still encounters the emulator's
missing mixer. The diagnostic build has been installed with verified file
readback and safe ejection; its physical API/VAL/MIN/MAX readback is pending.

## Physical Stage C checklist

Record operator/date, project and SDK commits, artifact hash, tape! firmware
version, initial source/gain/effective monitor state, headphones/line cabling,
external test source and level, DAW/OS, USB buses, rate/block size, and captures.
Use low levels and an independent physical input source. Stop tape/radio
playback, avoid an output-to-input cable loop, and disable DAW input monitoring
for capture-only checks. Test one signal source at a time so playback and input
feed-through cannot be confused. Keep the pedal disconnected for this cut.

| Check | Procedure and evidence required | Result |
| --- | --- | --- |
| Launch/readback | Note stock settings before launch. C1 must show them without changing them; compare LINE/MIC, effective monitor and gain. Record N/A if unavailable. | PARTIAL: initial C1 launch PASS; gain readback FAIL (50000%). Guarded build shows N/A (range). Stock comparison and diagnostic readback NOT TESTED. |
| Monitor ON / local input | Select LINE; feed a steady external signal. With USB playback stopped, BTN1 press requests ON. Confirm effective ON and physical input audible at tape!'s output. | NOT TESTED |
| Monitor OFF / local input | Keep the same physical signal. BTN1 press requests OFF. Confirm effective OFF and that local physical-input feed-through disappears. | NOT TESTED |
| Monitor OFF / USB playback | Stop the external input signal; keep effective monitor OFF. Play a distinct DAW/USB signal to tape!. Confirm it still reaches the physical output. | NOT TESTED |
| Monitor OFF / USB capture | Stop DAW playback; restore the external physical-input signal. Keep monitor OFF and DAW software monitoring OFF. Capture tape!'s USB input; confirm the physical input is present. | NOT TESTED |
| Input gain / USB capture | At fixed external-source and DAW gains, turn the encoder through at least three displayed percentages. Record the capture level at each and verify it follows the device setting. | NOT TESTED |
| Source LINE/MIC/back | With headphones connected, BTN2 press/release must not switch source. Hold to change LINE to MIC, demonstrate the selected input, then hold back to LINE. Record effective monitor state after each switch. | NOT TESTED |
| Effective-state refusal/external change | In an installed-firmware configuration that refuses monitor enable (documented example: MIC without headphones), verify UI remains effective OFF. Check that a jack/state change is reflected without another app action. Keep levels down. | NOT TESTED |
| Same-boot exit/relaunch | Deliberately set source/gain/monitor, record them, hold BTN4 to exit, inspect stock settings and relaunch. C1 must not restore old values or apply defaults. Repeat exit/launch. | NOT TESTED |
| Reboot | Record chosen source/gain/monitor; reboot and inspect stock settings before launch, then C1. SDK says source/gain persist and monitor intent is session-only, defaulting ON at boot; effective state can still be OFF because of source/jack conditions. Record actual behavior. | NOT TESTED |
| No unintended routing | Across all above transitions, verify no feedback, unexpected signal path or clipping. Record any anomaly and stop the affected audio test. | NOT TESTED |

Monitor OFF's effects on local feed-through, USB playback and USB capture must
be accepted separately. Do not fill one row from another row's result. These
are firmware-control tests, not acceptance of custom TAPP audio routing or a
pedal round trip.
