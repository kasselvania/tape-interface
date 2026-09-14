# Hardware session: 2026-09-14

## Installation

The operator reported USB Drive Mode active. macOS identified the connected
USB device as **tape!**, vendor **Bedtime**, and its storage as **Tape Storage**.
The writable ExFAT volume was mounted at `/Volumes/Untitled`; its `apps/`
directory was empty before installation.

- App source commit: `1b3b2a80afe52810cd4112596826b7579c7a7fcc`.
- SDK commit: `a9cae67124f4209833e3f63c1cd97b11a8f73070`.
- Local command: `PATH="$(brew --prefix llvm@18)/bin:$PATH" make check verify`.
- Build and both SDK verification runs: PASS, including 6/6 imports.
- Installed file: `apps/interface.tapp`, 3,072 bytes.
- SHA-256: `5d2a2b14cebe76b0b81d52365e8755f268e333dacfdb12e712d5a2ea5c42c956`.
- Installed-file readback: byte-for-byte match with the local build; SHA-256
  checked again after `sync`.
- Safe ejection: `diskutil eject /dev/disk5` reported success.

This verifies file installation through USB storage. It does not establish that
the firmware can launch the app or that audio passes through the device.

## Physical checks pending

| Check | Result |
| --- | --- |
| Installed firmware version from Setup -> About | NOT REPORTED |
| Launch from Setup -> Apps -> interface | NOT TESTED |
| Title and exit hint visible on the physical display | NOT TESTED |
| BTN4 hold exits to firmware UI | NOT TESTED |
| Relaunch after exit | NOT TESTED |
| External line input and direct monitoring OFF | NOT TESTED |
| Bitwig/pedal round trip at 48 kHz, 64/128/256 samples | NOT TESTED |

Continue with [the physical round-trip procedure](ROUND_TRIP_TEST.md) after the
app launch/exit checks. Record physical observations separately from host-side
file verification.
