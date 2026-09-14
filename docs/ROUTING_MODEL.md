# Routing reconnaissance

## Basis and scope

Resolved local and remote `main` before work:
`e55e753b4320eac46d2f0ac6e33f4006739cb098`. That commit and all existing
firmware-1.1.4 evidence are preserved. SDK remains
`a9cae67124f4209833e3f63c1cd97b11a8f73070`, API 1.8.

Goal: map observable sources, buses and sinks. Neither probe is a product
interface or effect. New buffer access is confined to the separate probes;
`tapp/interface/interface.c` remains unchanged. No USB descriptors, firmware
channel names, sample rate, device input gain, input selection or Monitor Flag
are changed by either probe. No recording is implemented by the probes.

The operator has now reported firmware **1.2.0 (`fa4b2c7f`)** and selected WET.
Both probes are installed with verified file readback; see the
[new session](HARDWARE_SESSION_1.2.0_2026-09-14.md). Routing results remain
NOT TESTED. Do not copy 1.1.4 results into that session. SDK verification checks
the pinned public contract, not the installed firmware's actual routing.

## Vocabulary

| Name | Kind | Direction / meaning |
| --- | --- | --- |
| USB Playback | Source transport | Host/Bitwig -> tape!, two channels; not a physical jack |
| USB Capture | Sink transport | tape! -> host/Bitwig, two channels; source inside firmware not established |
| Line Input | Physical source | External line signal entering tape! |
| Mic Input | Physical source | tape!'s microphone signal |
| TAPP Input Bus | Bus | `mixer_get_in()`, stereo interleaved float samples; contributors depend on role and firmware |
| TAPP Output Bus | Bus | `mixer_get_out()`, standalone engine contribution; downstream routing unknown |
| Analog Playback | Output path | Conversion/distribution of audio toward Speaker and/or Output Jack; not assumed identical to USB Capture |
| Speaker | Physical sink | Built-in speaker; stereo summing/selection unknown |
| Output Jack | Physical sink | Physical stereo output; test left/right separately |
| Monitor Flag | Control/readback | Effective `os_audio_get_monitor()` value; no architectural independence is implied |
| Internal tape playback | Source | Native tapehead playback, distinct from USB Playback |
| Internal tape recording | Sink | Native tape recording, distinct from USB Capture |

Historical documents retain their original wording. In new evidence, use
**Monitor Flag**, not “bypass” or “direct monitor” as a routing explanation.
L/R always names channels in the specified transport or bus.

## Proven observations, separated by firmware

### Firmware 1.1.4 (`e924784c`) — retained operator evidence

See [the original session](HARDWARE_SESSION_2026-09-14.md) and
[the retained audio-path findings](AUDIO_PATH_FINDINGS.md), as committed in the
basis above. These are operator reports; no retained multichannel capture proves
a particular internal connection.

- USB audio remained enumerated when the original UI-only app exited.
- C1 launched; corrected gain display was reported working near 5%.
- Encoder movement changed the public input-volume parameter. Diagnostic
  samples showed API=49/49.5 for values approximately 0.098/0.099 in range 0..2,
  rather than the normalized return described by the pinned header.
- With Line Input present and Monitor Flag ON, the operator reported signal
  at Speaker and USB Capture.
- With Monitor Flag OFF, Speaker sound disappeared and the operator explicitly
  confirmed Bitwig's input meter/recording also became silent.
- USB Playback -> Analog Playback was not independently isolated in that session.

These establish coupling in that setup. They do not establish whether USB
Capture taps a mix after Monitor Flag, whether a shared stage is gated/idled,
or whether firmware 1.2.0 behaves the same way.

### Firmware 1.2.0 — baseline reported, routing NOT TESTED

| Session field | Evidence |
| --- | --- |
| Installed firmware and build hash from About | Operator reports 1.2.0 (`fa4b2c7f`) |
| Operator/date and unit identification | Operator report 2026-09-14; host serial TP-HBWL4Z-049 |
| Exact app commits, artifact hashes and role | Installed from `05b96fb`; verified hashes/roles in new session; launch NOT TESTED |
| Bitwig/device configuration, channel mapping, rate/block size | NOT TESTED |
| Cables, Line Input/Mic Input, Speaker/Output Jack state | NOT TESTED |
| Monitor Flag, input gain, WET, tapehead levels/sends/transport | Standalone initial readback: Monitor Flag ON, Line Input selected; pre-install WET reported. Other settings and post-install WET pending |
| Captures and listening observations | P1: TAPP Input Bus peaks move with Monitor Flag ON and OFF. Speaker/USB Capture present ON, silent/flat OFF. Operator reports; no capture file, Output Jack untested |

Record each change and each result separately; retain failures. “Audible” is
not interchangeable with “present on a recorded USB Capture channel.”

On 1.2.0, the paired P1 observations establish that Monitor Flag OFF does not
remove Line Input from TAPP Input Bus in the tested standalone configuration.
It does silence Speaker and USB Capture when the probe writes zero TAPP Output
Bus. This narrows the coupling to a path after or parallel to the input-bus
observation; it does not prove USB Capture uses TAPP Output Bus or Analog Playback.

P3 established that a standalone left tone written to TAPP Output Bus reaches
Speaker and USB Capture left with Monitor Flag OFF. The operator later corrected
a left/right mode mix-up and reported residual activity in the other channel
for both tones. The initial right-flat report must not be treated as verified
isolation. The residual has not been shown to contain the tone rather than
background noise. Mic Input pickup was suggested by the operator but is unproven.
Sink reachability does not establish the internal tap, channel isolation or
USB Playback separation.

## Unknown routing edges

| Edge or control effect | What must be measured |
| --- | --- |
| Line Input / Mic Input -> TAPP Input Bus | Contributors, L/R mapping, device gain, Monitor Flag dependence |
| USB Playback -> TAPP Input Bus | Whether host playback is mixed with physical inputs before the callback |
| USB Playback -> Analog Playback | Whether it passes when Monitor Flag is OFF |
| TAPP Output Bus -> Analog Playback / USB Capture | Which destinations receive the standalone contribution |
| Line Input / Mic Input -> Analog Playback outside the callback | Whether zero TAPP Output Bus leaves physical feed-through |
| Analog Playback -> Speaker / Output Jack | Summing, jack insertion switching, gain and channel behavior |
| Internal tape playback -> USB Capture / Analog Playback | Whether the same mix feeds both and how Monitor Flag affects it |
| TAPP Input Bus -> internal tape recording | Source-role WET behavior versus standalone engine replacement |
| Source-role contribution -> USB Capture | Unknown even if native tape recording succeeds |
| USB Playback -> USB Capture | Unintended return, shared mix or selectable source; must not assume identity |

The pinned header deliberately omits global mixer routing setters. It offers no
USB Capture source selector or independent USB Playback buffer accessor.
`examples/recorder.c` describes reading TAPP Input Bus without echoing it and a
firmware line/USB input-to-output addition after its callback. The source-app
contract in `tapp_api.h` and the SDK README instead specifies in-place writes to
TAPP Input Bus with WET on. Neither description identifies every USB edge.

## No-code hardware matrix — firmware 1.2.0

Start with native firmware, probes unloaded, no analog output-to-input loop,
low playback levels, and one distinguishable source at a time. Disable Bitwig
input monitoring when observing USB Capture. Use existing native recordings or
a disposable tape segment; do not overwrite valuable material. Log which
physical sink is used. No firmware update or installation is implied by this doc.

| ID | Test / controlled variation | Observe independently | 1.2.0 result |
| --- | --- | --- | --- |
| N0 | Confirm About version/hash after update; record native settings | Version, gain, source, Monitor Flag, WET, transport | PARTIAL: 1.2.0 (`fa4b2c7f`), pre-install WET reported; remaining settings pending |
| N1 | Native Monitor Flag OFF/ON/OFF with steady Line Input, USB Playback stopped | Speaker, Output Jack, USB Capture L/R | NOT TESTED |
| N2 | Repeat N1 using C1's Monitor Flag control, with same settings, then exit C1 | Native flag equivalence, effective state, each sink | NOT TESTED |
| N3 | Disconnect Line Input; stop internal tape playback; send USB Playback L only | Output Jack L/R, Speaker, USB Capture L/R; repeat flag OFF/ON | NOT TESTED |
| N4 | Same as N3, USB Playback R only | Same independent sinks, swap only host channel | NOT TESTED |
| N5 | USB Playback stopped; Line Input disconnected; play known internal tape audio | Analog Playback and USB Capture with flag OFF/ON | NOT TESTED |
| N6 | Steady Line Input; flag OFF; USB Playback stopped; record a disposable native tape segment | Stop source, play the recording afterward; compare flag-ON recording separately | NOT TESTED |
| N7 | Feed Line Input left only, then right only with same level | USB Capture L/R and Output Jack L/R; log native stereo mix setting, flag OFF/ON | NOT TESTED |
| N8 | Repeat distinguishable L/R sources with Output Jack connected/disconnected | Speaker behavior versus Output Jack, flag readback, USB Capture continuity | NOT TESTED |
| N9 | Mic Input with Output Jack connected/disconnected; low levels | Effective Monitor Flag refusal, USB Capture, Speaker/Output Jack | NOT TESTED |

If USB Capture stops when Monitor Flag goes OFF, record that failure and continue
mapping isolated paths. Do not enable an analog feedback loop to work around it.

## Standalone `route_probe`

Artifact: `build/route_probe.tapp`; ordinary Apps launcher; engine-replacing
standalone role. Native tapehead behavior during this role is not inferred from
source-role behavior. It always fetches current bus pointers and sample count
inside `process()`, using the SDK's 48 kHz, interleaved L/R contract.

| Action | Behavior |
| --- | --- |
| Launch | METER INPUT / ZERO OUTPUT; disarmed; whole TAPP Output Bus zeroed each block |
| BTN1 press | Next mode; always disarms |
| BTN2 hold | Arm/disarm selected mode; METER INPUT / ZERO OUTPUT cannot arm |
| BTN2 short press/release | Does not arm |
| BTN3 press | METER INPUT / ZERO OUTPUT, starting at next processed block |
| Encoder | Anti-monitor trim only while disarmed, 1 percentage point per delta; no device gain writes |
| BTN4 hold | Disarm and exit; pause also disarms |

| Mode | Armed TAPP Output Bus behavior |
| --- | --- |
| METER INPUT / ZERO OUTPUT | Zero every sample; meter TAPP Input Bus L/R |
| TONE OUT L | 997 Hz, about -36 dBFS peak, left only; right exactly zero |
| TONE OUT R | 1499 Hz, about -36 dBFS peak, right only; left exactly zero |
| COPY INPUT TO OUTPUT | L/R unchanged at unity, not accumulated |
| ANTI-MONITOR / EXPERIMENTAL | Negative TAPP Input Bus x trim; default 10%, adjustable 0..25%; requires held arming |

Any unarmed mode writes zeros. Tone and anti-monitor arming show a large warning.
The peak display is the most recently processed block's absolute sample peak,
expressed in %FS (100%=1.0), before probe output writes. It is not a retained
peak or a USB/analog meter. The display saturates at 6553.5% for extreme/non-finite
samples; metering does not alter samples. COPY preserves samples even above 1.0;
it adds no gain but cannot promise unclipped physical sinks.

Anti-monitor is a bounded topology experiment. It has no delay alignment and
cannot be assumed to cancel any firmware signal. The 25% cap deliberately does
not attempt full-amplitude cancellation. Measure any attenuation, reinforcement,
phase or channel effect at each sink; never label it successful cancellation
without captures. No code accesses the firmware's hidden routing controls.

Neither probe changes Monitor Flag. Establish each flag condition in native UI
(or C1), then launch the probe and confirm its effective readback. Do not claim
zero probe output means silent Speaker, Output Jack or USB Capture: parallel
firmware contributions may remain. A user-held action is required before any
probe-generated tone, copy or negative-input contribution is enabled.

## Source `route_source_probe` — firmware 1.2.0 only

Artifact: `build/route_source_probe.tapp`; `TAPP_DECLARE_TYPE(AppTypeSrc)` is
stamped into the manifest. Select it from the native source-slot picker, not
the ordinary Apps launcher. **Enable WET in the native UI** before the test.
The public SDK has no WET setter/readback; the displayed WET instruction is a
prerequisite, not a claim that WET was enabled. Log the operator's native setting.

It starts disabled, meters TAPP Input Bus and writes zeros to that entire bus.
BTN1 hold enables/disables identity pass-through: each L/R sample is written
back unchanged to TAPP Input Bus. BTN2 press returns to zero. It never accesses
TAPP Output Bus or tapehead sends, adds no effects and changes no input gain.
Native input-level scaling and other firmware contributions may still apply.

The source role preserves the native tape engine. BTN4/BTN5 holds are reserved
for firmware navigation: hiding the UI does **not** unload or stop the source.
Stop it with BTN2 before leaving, or unload it in the source picker. Pass-through
in the background is part of the source contract, not inferred from emulator
navigation. Old firmware may ignore the role and load it as standalone, so do
not test this role on the retained 1.1.4 baseline.

**USB Capture behavior remains unknown until hardware testing**, even if WET
samples are visible or internal tape recording works. Input-bus pass-through
alone does not establish USB Capture independence from Monitor Flag.

## Probe evidence matrix — firmware 1.2.0

For each row record Monitor Flag, source, WET, exact artifact, input stimulus and
levels, separate USB Capture L/R recordings and the observed physical sink.
No probe result may be filled from native/emulator results or a different row.

| ID | Probe experiment | 1.2.0 result |
| --- | --- | --- |
| P0 | Standalone launch/relaunch disarmed; effective flag/source and input peaks | PARTIAL: launch screen shows METER INPUT / ZERO OUTPUT, Monitor Flag ON, LINE, peaks L0/R0. Relaunch and stimulated meter response pending. |
| P1 | METER INPUT / ZERO OUTPUT, Line Input, flag OFF/ON; observe each sink | OBSERVED: input peaks move ON/OFF; Speaker and USB Capture present ON, silent/flat OFF. Output Jack and isolated L/R NOT TESTED. |
| P2 | Same, USB Playback L then R; Line Input disconnected | NOT TESTED |
| P3 | Held TONE OUT L then ZERO; record USB Capture L/R and each physical sink | Speaker/USB Capture signal observed. Later correction reports residual other-channel activity; initial right-flat report is superseded. Isolation UNRESOLVED; Output Jack untested. |
| P4 | Held TONE OUT R then ZERO; same measurements | Operator confirms right mode tried after correcting a prior left-mode mistake; reports residual activity with both tones. Isolation, baseline and meter identity UNRESOLVED; no measured capture. |
| P5 | Held COPY INPUT TO OUTPUT vs ZERO, isolated Line Input L/R, flag OFF/ON | NOT TESTED |
| P6 | EXPERIMENTAL negative-input trim 0/10/25%, rearm each; compare captured levels/phase with ZERO | NOT TESTED |
| P7 | Standalone pause/exit/relaunch; native tape/flag/source behavior | NOT TESTED |
| S0 | Source role selected, native WET confirmed; disabled meter/zero | NOT TESTED |
| S1 | Held source identity pass, Line Input L then R; internal tape recording and USB Capture separately | NOT TESTED |
| S2 | Held source identity pass, USB Playback L then R; flag OFF/ON | NOT TESTED |
| S3 | Source hide/reopen, BTN2 stop, picker unload; check native tape continuity | NOT TESTED |

## Local validation and limits

From the repository root:

```sh
PATH="$(brew --prefix llvm@18)/bin:$PATH" make test check verify
```

This builds all three apps, runs the existing C1 native tests and the separate
routing tests, runs the SDK verifier during each build and again explicitly,
and checks exact per-app import sets. C1 retains its original 18-import boundary.
The standalone probe has 22 public imports; source has 13, without
`mixer_get_out`. Neither probe imports USB descriptor/routing setters or FX
buffer access. Firmware source code is not modified.

Additional host check:

```sh
clang -std=c11 -O2 -ffast-math -Wall -Wextra -Werror -U__ARM_FP -I tape_sdk \
  tests/route_probe_native.c -lm -o build/route_probe_native_fast
./build/route_probe_native_fast
```

Tests cover mode transitions/held arming, zero default, disarming on mode change,
pause/exit, complete odd/even output writes, null buses/context, alias-safe unity
copy, L/R peaks, tone frequency/amplitude/RMS/isolation across blocks, bounded
anti trim, source identity and role-specific navigation/lifecycle. UI/audio
state is exchanged with lock-free 32-bit atomics; audio processing contains no
UI calls, allocation or file access.

Optional official emulator UI/lifecycle harness, using the same official runtime
setup documented in [Stage C1](STAGE_C1.md):

```sh
node tests/routing_emulator.mjs
```

This checks rendered mode/arm/zero transitions, warnings and standalone
exit/reload; source zero/pass/stop UI is checked where supported. It records
runtime and artifact hashes in ignored `build/emulator/routing-results.json`.
It does **not** prove USB Playback, USB Capture, Speaker, Output Jack, internal
tape recording, WET sourcing or source-role background behavior on hardware.

### Recorded local result — 2026-09-14

Homebrew LLVM 18.1.8: `make test check verify` above passed. Both SDK verifier
runs passed for every artifact (18/18, 22/22 and 13/13 imports respectively).
The original nine C1 native groups and five routing-probe groups passed. The
routing suite also passed with `-O2 -ffast-math` and separately with AddressSanitizer
and UndefinedBehaviorSanitizer. The official emulator UI harness passed with
no unresolved symbols; visual inspection confirmed the tone/anti warnings and
source pass/stop instructions fit. These results are software checks only.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `build/interface.tapp` (unchanged) | 5,468 | `89c60b0b0e02ce0a3729148df5d9f637509abaaca9150ccf109efa7285ebace3` |
| `build/route_probe.tapp` | 8,916 | `c9f3aeeb4dc9ceaa9518a6eff6bf78d947a2eaab6a95bd80ca8ba82f4fe4ea68` |
| `build/route_source_probe.tapp` | 5,976 | `40c9648b5c1f9dae1aaf895de101fba821e5be1f979fa09613e6101c71a7a265` |

Emulator runtime SHA-256:

- JS: `e1ebfc45f9053fafcaa699466e29bc0da59e2eceb62e1624df864b22d38c7b4c`.
- WASM: `1a06209fbe86b16acc79271d2787048bfdcd45732fd4a425e46fa27e7fcf6d64`.

No probe was installed on hardware during this implementation. Verify firmware
1.2.0's installed About readback and establish N0 before filling any evidence row.
