# Tape Interface

Stage C1 is a fullscreen **firmware audio-control probe**. It displays the
effective monitor state, LINE/MIC source, and device input-gain percentage, and
lets the operator change those stock firmware settings. It has no audio
callbacks, DSP, recording, USB changes, custom bypass, or routing logic.

| Control | Firmware action |
| --- | --- |
| BTN1 press (`mon`) | Toggle direct monitoring; display the effective result |
| BTN2 hold (`src`) | Switch LINE/MIC; this device setting is persistent |
| Encoder | Adjust device input gain through its parameter API and callback; persistent |
| BTN4 hold (`exit`) | Exit without restoring or changing audio settings |

Initialization only reads audio settings. The UI follows external state changes,
including firmware refusing a monitor-enable request. Unavailable input gain
shows `N/A`. The screen explicitly identifies **STOCK ROUTE / NO DSP**.

Current hardware finding: **monitor OFF also silences Bitwig's input
meter/recording** on firmware 1.1.4 (`e924784c`). C1 therefore has not established
independent local monitoring and USB capture. See [audio-path findings and
remaining questions](docs/AUDIO_PATH_FINDINGS.md) before further routing work.
The pedal round trip is blocked by that requirement failure in the tested setup.

The corrected gain display is operator-confirmed at approximately 5%; exact
4.95% text was not reported. Encoder movement changed the underlying parameter.
Gain-versus-capture-level testing remains pending. The SDK percentage discrepancy
and its checked conversion are documented in [Stage C1](docs/STAGE_C1.md).

## Future intended signal path

```text
Bitwig Hardware FX
-> tape! USB output
-> tape! physical output
-> external pedal
-> tape! physical input
-> tape! USB input
-> Bitwig Hardware FX return
```

USB output/input are named from Bitwig's perspective (send/return).
**Bitwig's master must use a separate monitor output. Keep tape! direct input
monitoring OFF:** monitoring the returned pedal signal back to tape!'s output
would close a feedback loop.

This path is not implemented or proven by C1. The operator has established that
the audible input feed-through is stock firmware monitoring; the earlier
screen-only app did not implement it, and USB audio remains available after
exiting that app. The [pedal round-trip test](docs/ROUND_TRIP_TEST.md) is deferred
until the firmware's independent monitor/USB behavior has been established.

## Local build

[bedtime-llc/tape_sdk](https://github.com/bedtime-llc/tape_sdk) is a git submodule
pinned to `a9cae67124f4209833e3f63c1cd97b11a8f73070` (API 1.8).

```sh
git clone --recurse-submodules https://github.com/kasselvania/tape-interface.git
cd tape-interface
# For an existing clone:
git submodule update --init --recursive
```

Requires Make, Python 3, and LLVM clang/ld.lld/binutils (15+). On macOS, use
Homebrew LLVM 18; Apple's clang alone is insufficient. This app has no bitmap
assets, so ImageMagick is unnecessary.

```sh
brew install llvm@18
export PATH="$(brew --prefix llvm@18)/bin:$PATH"
make check test verify
```

On Debian/Ubuntu, install `make python3 clang lld llvm` and run
`make check test verify` with those tools on PATH. The SDK verifier needs LLVM
binutils on PATH even when the SDK builder discovers a Homebrew keg itself.

`make test` runs native firmware-stub checks against the actual app callbacks.
`make verify` always builds `build/interface.tapp`, runs the SDK's automatic gate,
then calls `tape_sdk/tools/verify-tapp.sh` and checks the finished binary's C1
control-only imports. `make build` builds with the automatic SDK gate;
`make clean` removes the app and native test executable. Build output is
ignored by Git. To try the app on tape!, copy the artifact to its `apps/` folder
in USB Drive Mode, leave drive mode, and launch it from Setup -> Apps.

## Validation

Successful C1 command from the repository root on 2026-09-14:

```sh
PATH="$(brew --prefix llvm@18)/bin:$PATH" make test check verify
```

Homebrew LLVM 18.1.8 produced the corrected `build/interface.tapp` (**5,468 bytes**).
All nine native firmware-stub groups and both SDK verification runs passed,
including **18/18 imports resolved**. The finished artifact imports no engine or
audio-buffer-processing functions.

The [official browser emulator](https://tapp.b.edti.me/) loaded the binary and
displayed the C1 UI. The same unmodified official WASM runtime passed scripted
monitor/source gestures, exit and reload. It lacks `mixer_get`, so gain shows
`N/A`; real gain control is not proven by that emulator. See
[the detailed results and reproduction commands](docs/STAGE_C1.md).

Earlier two-cycle hardware launch/exit acceptance applies to the **3,072-byte
UI-only scaffold**. Initial C1 launch and the failed gain readback are recorded
separately in [the hardware session](docs/HARDWARE_SESSION_2026-09-14.md).
