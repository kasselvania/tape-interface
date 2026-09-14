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

## Physical checks

The operator confirmed the title/exit-hint check and reported entering and
exiting the app twice. These are operator-observed hardware results; no physical
display capture was collected.

| Check | Result |
| --- | --- |
| Installed firmware version from Setup -> About | NOT REPORTED |
| Launch from Setup -> Apps -> interface | PASS, operator report |
| Title and exit hint visible on the physical display | PASS, operator report |
| BTN4 hold exits to firmware UI | PASS, operator report; two cycles |
| Relaunch after exit | PASS, operator report |
| External line input and direct monitoring OFF | NOT TESTED |
| Bitwig/pedal round trip at 48 kHz, 64/128/256 samples | NOT TESTED |

## Host audio enumeration and handoff

After Drive Mode ended, `system_profiler SPAudioDataType` reported **tape!**,
manufacturer **Bedtime**, USB transport, two input channels, two output channels,
and a current sample rate of **48,000 Hz**. The app's running/exited state at the
time of this host snapshot was not established. Enumeration does not prove audio
routing or sound.

Bitwig's Audio settings visibly showed the existing **8A** device selected at
**48,000 Hz / 128 samples**. No combined-device configuration or audio test was
completed. The operator will configure Bitwig and the physical connections.

Continue with [the physical round-trip procedure](ROUND_TRIP_TEST.md). Firmware
version, pedal, monitor destination, input source, and monitoring OFF still need
to be recorded before the audio test.
