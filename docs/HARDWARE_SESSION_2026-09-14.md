# Hardware session: 2026-09-14

The installation and launch/exit results below belong to the original UI-only
scaffold. They do not accept the subsequent C1 firmware-control probe.

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
**48,000 Hz / 128 samples**. At that snapshot, no combined-device configuration
or audio test had been completed. The operator took over Bitwig configuration
and the physical connections.

The proposed next step was [the physical round-trip procedure](ROUND_TRIP_TEST.md),
with firmware version, pedal, monitor destination, input source, and monitoring
OFF still unrecorded. The subsequent observations below supersede that next step.

## Subsequent operator observations and C1 scope

The operator reported audible audio with USB connected, both 3.5 mm audio jacks
empty, and HW FX at 100% Mix. The operator also confirmed that exiting the
interface app did not remove tape! as a USB audio device. The operator has since
established the audible input-to-output feed-through as **stock firmware
monitoring**, not routing or bypass implemented by the TAPP. The scaffold had
no audio callbacks or routing code.

These reports do not independently establish the effect of monitor OFF on USB
playback, USB capture, or input gain. Those are separate **NOT TESTED** rows in
[the Stage C1 checklist](STAGE_C1.md). C1 exposes stock monitor/source/gain
controls without taking audio-buffer ownership. Pedal testing remains deferred.

## C1 installation

After the operator activated USB Drive Mode again, macOS identified Bedtime
**tape!** and its **Tape Storage** disk as `disk5`, with writable ExFAT partition
`disk5s1` mounted at `/Volumes/Untitled`.

- Installed C1 app commit: `a0ca8c87f69040b2f370d9449e4740a663dc50d8`.
- SDK remains `a9cae67124f4209833e3f63c1cd97b11a8f73070`.
- Replaced `apps/interface.tapp`: original scaffold hash and 3,072-byte size
  verified before replacement; backed up locally under ignored
  `build/hardware-backup/20260914-103847/`.
- Installed C1 file: **4,896 bytes**; readback matched the local artifact
  byte-for-byte after `sync`.
- SHA-256: `c46db5c6e624aef2369fe83f12dd1d3106b6a3605c581d6c9724b89889022fb5`.
- Backed up and removed this app's old macOS AppleDouble sidecar. The device's
  `apps/` directory contained only `interface.tapp` after installation.
- `diskutil eject /dev/disk5` completed successfully.

This records C1 file installation only. C1 launch, displayed settings and every
physical control/audio result in [the Stage C1 checklist](STAGE_C1.md) remain
**NOT TESTED** pending operator reports.
