# Hardware session: firmware 1.2.0, 2026-09-14

This is a new session. Firmware-1.1.4 observations in
[the earlier session](HARDWARE_SESSION_2026-09-14.md) remain unchanged and do not
establish results for this firmware.

## Operator baseline

- Operator reported About firmware **1.2.0 (`fa4b2c7f`)**.
- Operator reported selecting **WET** before activating USB Drive Mode.
- WET after leaving USB Drive Mode has not yet been rechecked.
- Host observed Bedtime **tape!**, serial `TP-HBWL4Z-049`, exposing Tape Storage.
- Current Monitor Flag, Line Input/Mic Input selection, gain, cabling,
  Speaker/Output Jack state, tapehead transport/sends, and Bitwig ports/rate/block
  size have not yet been reported for this session.

## Probe installation

At installation, local and remote main both resolved to
`05b96fbe602e4a54cd9876d5d0ffe3873cb7a74a`. SDK remained
`a9cae67124f4209833e3f63c1cd97b11a8f73070`, API 1.8.

On 2026-09-14 at approximately 14:09 PDT, the host identified Tape Storage as
`disk4`, writable ExFAT `disk4s1`, mounted at `/Volumes/Untitled`.
The two probe filenames were absent before installation.

| Installed file | Role | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `apps/route_probe.tapp` | Standalone engine | 8,916 | `c9f3aeeb4dc9ceaa9518a6eff6bf78d947a2eaab6a95bd80ca8ba82f4fe4ea68` |
| `apps/route_source_probe.tapp` | AppTypeSrc | 5,976 | `40c9648b5c1f9dae1aaf895de101fba821e5be1f979fa09613e6101c71a7a265` |

Both destination files matched the local artifacts byte-for-byte after `sync`.
The existing `apps/interface.tapp` was preserved byte-for-byte. The apps directory
contained these three TAPPs and no extra installation files afterward.
`diskutil eject /dev/disk4` completed successfully.

This is file-installation evidence, not probe launch or routing evidence.

## Next observation

Leave USB Drive Mode, recheck WET, then open `route_probe` from Apps. Do not hold
BTN2 to arm yet. Report the mode, Monitor Flag, LINE/MIC label, and input peaks.
The expected probe mode is METER INPUT / ZERO OUTPUT, disarmed. Zero TAPP Output
Bus does not guarantee device-wide silence through other firmware paths.

The source probe is selected later through the native source-slot picker;
its WET prerequisite and background lifecycle are documented in
[the routing model](ROUTING_MODEL.md). Do not infer its behavior from launching
the standalone probe.

## Physical results

Native routing rows N1–N9, standalone rows P2–P7 and source rows S0–S3 in
[the routing matrices](ROUTING_MODEL.md) remain **NOT TESTED**. N0, P0 and P1
are partial as recorded below. Speaker and USB Capture signal presence with
Line Input and Monitor Flag ON are operator-observed; Output Jack, USB Playback
and internal tape behavior remain untested.

## Standalone initial screen readback

The operator reported:

```text
Meter input / zero output
monitor flag: ON LINE
input peak L0 R0
```

Standalone launch and the initial displayed mode/state are confirmed by operator
report. Monitor Flag is ON; Line Input is selected. Relaunch and the explicit
disarmed label have not been separately reported. Zero displayed peaks alone
do not prove callback activity, an absent input, a silent TAPP Output Bus or
any sink routing. Source stimulus/cabling, WET after disk mode and Bitwig state
at this readback were not supplied.

Next: keep METER INPUT / ZERO OUTPUT, stop USB Playback, feed a steady external
Line Input at low level, and observe the TAPP Input Bus peaks, Speaker or
Output Jack, and USB Capture separately. Keep DAW input monitoring off. This
begins P1 with Monitor Flag ON; no arming or generated probe signal is required.

## P1: Line Input with Monitor Flag ON

Responding to that test, the operator reported:

- TAPP Input Bus L/R peaks move.
- Speaker is audible.
- Bitwig's USB Capture input meter moves.
- Output Jack was not checked.

The established probe configuration was METER INPUT / ZERO OUTPUT, Monitor Flag
ON, LINE. The instructions were to stop USB Playback and disable Bitwig input
monitoring; those host controls were not separately captured. No numerical peak
values, isolated L/R stimuli or audio recording were supplied.

This supports Line Input reaching TAPP Input Bus, Speaker and USB Capture while
the probe is programmed to write zeros to the whole TAPP Output Bus. A firmware
contribution after or parallel to that write is a supported inference, not a
measurement of the exact connection. Do not infer Output Jack behavior or USB
Capture's exact tap point from these reports.

Next: set Monitor Flag OFF through C1 (the probe has no Monitor Flag setter),
then relaunch `route_probe` and confirm METER INPUT / ZERO OUTPUT, flag OFF,
LINE. Keep the external signal and gain unchanged. Observe the same three
results again: TAPP Input Bus peaks, Speaker, USB Capture meter. This tests
whether the flag changes the input seen by the probe or only downstream paths.

## P1: Line Input with Monitor Flag OFF

The operator confirmed METER INPUT / ZERO OUTPUT, Monitor Flag OFF, LINE,
then reported with audio present:

- Input peak meters move.
- No audio from Speaker.
- Bitwig audio meters are flat, with no audio.

This is the paired Monitor Flag OFF observation, following the requested C1
flag change and return to the standalone probe. It establishes that Line Input
is still available at TAPP Input Bus while Speaker and USB Capture are silent
in this configuration. The loss cannot be explained solely by stopping Line
Input before it reaches that bus. A downstream or parallel firmware path changes;
the exact USB Capture tap and internal routing are not established.

| METER INPUT / ZERO OUTPUT | TAPP Input Bus peaks | Speaker | USB Capture meter |
| --- | --- | --- | --- |
| Monitor Flag ON | Moving | Audible | Moving |
| Monitor Flag OFF | Moving | Silent | Flat |

Both rows are operator reports, not saved signal measurements. Output Jack,
isolated stereo channels, and Mic Input remain untested. Do not generalize this
result to USB Playback or source-role processing.

Next: stop external Line Input audio and USB Playback, keep Monitor Flag OFF,
select TONE OUT L, and explicitly hold BTN2 to arm the approximately -36 dBFS
997 Hz test. Observe Speaker and USB Capture L/R independently, then BTN3 ZERO.
This begins P3 and tests destinations of TAPP Output Bus without conflating them
with Line Input feed-through. No new installation is needed.

## P3: TONE OUT L with Monitor Flag OFF

In response to the left-tone test, the operator reported Speaker audible,
USB Capture left meter moving, USB Capture right meter not moving, and BTN3
returning the probe to zero mode. The test condition was Monitor Flag OFF with
external Line Input audio and USB Playback stopped as instructed. No numerical
levels, recorded frequency measurement or Output Jack observation was supplied.

This establishes the standalone TAPP Output Bus left contribution reaching
Speaker and USB Capture left with Monitor Flag OFF, with no right-channel meter
activity observed. It does not locate the exact USB Capture tap, quantify
crosstalk, prove Output Jack routing or establish USB Playback separation.
The programmed tone is 997 Hz at approximately -36 dBFS peak; those values are
software-validated, not independently measured at the physical sinks here.
BTN3's return to zero mode is operator-confirmed; post-stop sink levels were not
separately reported.

Next: from METER INPUT / ZERO OUTPUT, press BTN1 twice to select TONE OUT R /
1499 Hz, hold BTN2 to arm, observe Speaker and both USB Capture meters, then
BTN3 ZERO. Leave Monitor Flag OFF and the other sources stopped.

## P4 attempt: unexpected meter balance

During the requested right-tone test, the operator reported Speaker audible
and Bitwig bars showing mainly left activity with some right activity. This
does not match the expected right-only result.

The exact on-device mode/Monitor Flag readback and the identity of the Bitwig
meter being viewed were not reconfirmed with this report. Record it as an
unresolved observation, not proof of reversed channels, crosstalk, firmware
mixing or a successful right-channel test. No audio capture was supplied.

Next: confirm the displayed mode/Monitor Flag and whether the observed bars
belong to USB Capture input, a track, or the master. Return to ZERO with BTN3.
Resolve those details before changing routing or accepting channel isolation.
