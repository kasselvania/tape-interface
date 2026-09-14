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

Native routing rows N1–N9, standalone rows P1–P7 and source rows S0–S3 in
[the routing matrices](ROUTING_MODEL.md) remain **NOT TESTED**. N0 and P0 are
partial as recorded below. No USB Playback, USB Capture, Analog Playback,
Speaker, Output Jack or internal tape behavior has been accepted in this session.

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
