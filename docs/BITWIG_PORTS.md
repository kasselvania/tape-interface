# Bitwig port names for routing reconnaissance

These are recommended **host-side aliases**, not firmware channel renames or
USB descriptor changes. The operator configures Bitwig. C1 and the probes do
not change host ports or establish their physical mapping.

| Alias | Bitwig direction | Intended channel |
| --- | --- | --- |
| Tape PB L | Output | USB Playback left: host/Bitwig -> tape! |
| Tape PB R | Output | USB Playback right: host/Bitwig -> tape! |
| Tape CAP L | Input | USB Capture left: tape! -> host/Bitwig |
| Tape CAP R | Input | USB Capture right: tape! -> host/Bitwig |
| Tape Playback | Stereo output | Tape PB L + Tape PB R, in that order |
| Tape Capture | Stereo input | Tape CAP L + Tape CAP R, in that order |

In Bitwig's audio input/output bus configuration, bind the mono and stereo aliases
to the two channels belonging to tape!. When using an aggregate device, resolve
tape!'s channel offsets from the actual host configuration; do not assume they
are channels 1/2. Record native channel labels, offsets, device/aggregate name,
sample rate and block size in the new session before assigning test results.
Do not infer physical left/right identity from an alias alone: verify it using
left-only and right-only stimuli in [the routing matrix](ROUTING_MODEL.md).

Use Tape Playback only for sending a known DAW signal into USB Playback. Use
Tape Capture to record what USB Capture supplies. USB Capture is **not** yet
proven to mean isolated Line Input or Mic Input. Do not label it “Line In” until
that edge is established. Keep other playback and software input monitoring off
when testing a single source; listen to recordings afterward through an
independent host output. Do not build a hardware-effects loop during this mapping.

Firmware 1.2.0 mapping worksheet:

| Alias | Host channel(s) | Isolated stimulus/capture evidence | Result |
| --- | --- | --- | --- |
| Tape PB L | Awaiting operator | Left-only playback | NOT TESTED |
| Tape PB R | Awaiting operator | Right-only playback | NOT TESTED |
| Tape CAP L | Awaiting operator | Recorded left channel with isolated stimuli | NOT TESTED |
| Tape CAP R | Awaiting operator | Recorded right channel with isolated stimuli | NOT TESTED |
| Tape Playback | Awaiting operator | Stereo order check | NOT TESTED |
| Tape Capture | Awaiting operator | Stereo separation check | NOT TESTED |
