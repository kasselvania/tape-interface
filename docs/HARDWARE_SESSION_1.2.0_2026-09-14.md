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

All native routing rows N1–N9, standalone rows P0–P7 and source rows S0–S3 in
[the routing matrices](ROUTING_MODEL.md) remain **NOT TESTED**. N0 is partial:
firmware/build and pre-install WET are operator-reported, remaining settings
are pending. No USB Playback, USB Capture, Analog Playback, Speaker, Output Jack
or internal tape behavior has been accepted in this new session.
