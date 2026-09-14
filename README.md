# Tape Interface

Initial buildable scaffold for using tape! with Bitwig Hardware FX. The app is
adapted from the SDK's [smallest example](tape_sdk/examples/simple_app.c): it
displays **Tape Interface** and exits through `os_app_exit()` when BTN4 is held.
It has no audio callbacks, effects, recording, USB changes, or routing logic.

## Intended signal path

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

This path is an intended hardware configuration, not a capability implemented or
proven by this screen-only app. Physical acceptance requires actual tape!
hardware, Bitwig, and a pedal. Follow [the round-trip test](docs/ROUND_TRIP_TEST.md).

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
make check
make verify
```

On Debian/Ubuntu, install `make python3 clang lld llvm` and run `make check` and
`make verify` with those tools on PATH. The standalone SDK verifier needs LLVM
binutils on PATH even when the SDK builder discovers a Homebrew keg itself.

`make verify` always builds `build/interface.tapp`, runs the SDK's automatic gate,
then calls `tape_sdk/tools/verify-tapp.sh` explicitly. `make build` builds with the
automatic gate; `make clean` removes the generated artifact. Build output is
ignored by Git. To try the app on tape!, copy the artifact to its `apps/` folder
in USB Drive Mode, leave drive mode, and launch it from Setup -> Apps.

## Validation

Successful command from the repository root on 2026-09-13:

```sh
PATH="$(brew --prefix llvm@18)/bin:$PATH" make check verify
```

On macOS 26.4.1 arm64 with Homebrew LLVM 18.1.8, this produced
`build/interface.tapp` (**3,072 bytes**). Both SDK verification runs passed:
ELF32 little-endian ARM relocatable, supported relocations, no unwind tables,
valid manifest/entry point, no 64-bit floating-point instructions, and **6/6
imports resolved** against the pinned SDK header. No gate was skipped.

The [official browser emulator](https://tapp.b.edti.me/) loaded that local binary
and visibly displayed the title and exit hint. A temporary native callback smoke
check with firmware stubs passed init/deinit and all 15 button/state combinations:
only BTN4 HOLD requests exit. That host check used `-U__ARM_FP` to select the
header's portable math branch; the actual ARM build used the unmodified SDK flags.
Emulator hold-to-exit was not verified. On 2026-09-14, the operator confirmed the
physical title/exit hint and two successful launch/BTN4-hold-exit cycles. macOS
also enumerated tape! as a stereo USB audio device at 48 kHz. See the
[hardware session](docs/HARDWARE_SESSION_2026-09-14.md). The physical Bitwig/pedal
audio round trip remains **NOT TESTED**.
