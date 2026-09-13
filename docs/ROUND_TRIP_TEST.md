# Physical round-trip test

Status: **NOT TESTED on tape! hardware with Bitwig and a pedal.** Local compilation
or emulator operation cannot establish USB/analog routing, sound, latency, or
feedback safety. The app is only a screen; it does not configure this path.

## Physical cabling and safety

1. Stop Bitwig transport, mute the send, and turn monitor and tape! output levels
   down before connecting anything. Stop tape! playback/recording and radio.
2. Connect tape!'s USB-C port to the Bitwig computer with a data cable. Connect a
   separate monitor interface/output to headphones or speakers; never monitor
   Bitwig's master through tape! for this test.
3. Connect tape!'s 3.5 mm stereo line/headphone output to the pedal input, and
   the pedal output to tape!'s 3.5 mm stereo line input. Use stereo TRS-to-dual-TS
   breakout cables as appropriate. For a mono pedal, use only each breakout's
   left leg and the matching mono DAW ports; leave right legs unconnected. Do not
   use a passive summing cable or treat a stereo TRS jack as balanced mono.
4. Select tape!'s external line input, not its microphones. Set channel mapping
   explicitly. Start the pedal bypassed and use conservative levels; a pedal
   that needs instrument-level input may need attenuation/reamping.
5. **Keep tape! direct input monitoring OFF for the entire test.** The pedal
   return must not feed the physical send again. The manual describes monitoring
   in the REC context menu (hold REC, then toggle monitoring with the right-arrow
   button); inspect the actual OFF state on the installed firmware before
   unmuting. The scaffold does not enforce or change this setting.
6. Check OFF again after app launch/exit or any device mode change. If the state
   cannot be confirmed, keep the send muted. Stop immediately if levels rise
   unexpectedly, howl, or clip; mute the send before changing cables/settings.

## Bitwig combined device and 48 kHz reference

On macOS or Linux, open Settings -> Audio -> Selected Device(s) -> Create new
Combined Device. Include tape! and the separate monitor device. Choose one clock
master (use the monitor interface for this reference) and record it. Set the
combined device and both members to **48,000 Hz**, verifying the actual rate.
Record any resampling/drift correction in use. If 48 kHz or a requested block
size is unavailable, mark that configuration unsupported; do not substitute a
different value. Bitwig's built-in combined-device option is documented for
macOS/Linux; a Windows multi-device driver setup needs separate documentation.

Define and record these buses and their physical channel numbers:

| Bitwig bus | Physical destination/source |
| --- | --- |
| Monitor | Separate monitor device output; master output goes here |
| Pedal Send | tape! USB output -> tape! physical output -> pedal input |
| Pedal Return | Pedal output -> tape! physical input -> tape! USB input |

Use a test audio track with Hardware FX (HW FX). Set its send to Pedal Send and
return to Pedal Return, with a fully wet return and no additional processing.
The track goes to the master, and the master goes only to Monitor. Remove other
routes to tape! outputs. Use mono buses for the mono cabling above; test each
channel separately for stereo. Start at **128 samples**, then repeat at 64 and
256. These are host block sizes, not changes to the tape! firmware's audio block.

Record: date/operator; OS and Bitwig versions; tape! model/firmware; project and
SDK commits; whether the scaffold is running or exited; monitor device/driver;
clock master; actual rate/block size; USB connection; pedal model, power, bypass
mode, controls and levels; cables; send/return port numbers. A result with the app
exited establishes only that firmware/DAW configuration, not operation while the
app is running.

## Latency procedure

1. Save the configured Bitwig project and screenshots of the audio device/buses,
   master routing, HW FX settings, and tape! monitoring OFF. Establish a quiet,
   low-level return with the pedal bypassed before measuring. No return is a
   failed test, not a reason to enable direct monitoring.
2. At each actual 48 kHz / 64, 128, or 256-sample setting, run HW FX's latency
   detection with the pedal bypassed three times. Record every reported value,
   unit, and whether it includes driver-reported latency. If that Bitwig version
   cannot detect latency, record unavailable rather than guessing from block size.
3. Apply the detected compensation. Play several isolated, low-level clicks with
   silence between them. Capture the HW FX output and a dry reference from the
   same source to separate Bitwig tracks in the same pass, with no additional
   processing or manual clip alignment. Save the source, returned audio and
   project so the timing can be inspected. This is DAW evidence capture, not
   recording functionality in the tapp.
4. At sample-level zoom, measure corresponding transient onsets:
   `residual_samples = returned_onset - reference_onset`. Report the median and
   range over at least three clicks; `milliseconds = samples / 48` at 48 kHz.
   Keep the detector's reported latency and the compensated residual separate.
   A DAW-aligned capture is not an uncompensated physical round-trip measurement;
   do not relabel it as one or assume `2 * block size` is the round-trip latency.
5. Run audio for at least 60 seconds per buffer size and record dropouts, clipping,
   unexpected feedback or timing drift. Then engage the pedal at conservative
   settings and confirm its audible return. Record bypassed versus engaged
   results separately; delays/reverbs/modulation can obscure transient timing.
   Save short audio evidence and note whether any digital pedal latency changed.

## Evidence table

Fill only from an actual hardware session. Link the saved project, screenshots
and audio files in the final column; leave missing measurements as NOT TESTED.
No buffer setting is accepted solely because its build or emulator test passes.

| Requested block (48 kHz) | Actual rate/block; app state | HW FX latency, 3 trials (unit/basis) | Compensated residual (samples/ms; range) | 60 s stability; pedal bypass/engaged; feedback | Evidence / result |
| --- | --- | --- | --- | --- | --- |
| 64 samples | NOT TESTED | NOT TESTED | NOT TESTED | NOT TESTED | NOT TESTED |
| 128 samples | NOT TESTED | NOT TESTED | NOT TESTED | NOT TESTED | NOT TESTED |
| 256 samples | NOT TESTED | NOT TESTED | NOT TESTED | NOT TESTED | NOT TESTED |

## References

- [Bedtime tape! specifications](https://b.edti.me/projects/tape/): physical ports.
- [tape! manual](https://b.edti.me/projects/tape/manual/): input monitoring and USB audio; check against the installed firmware.
- [Bitwig audio settings and combined devices](https://www.bitwig.com/userguide/latest/the_dashboard/).
- [Bitwig Hardware FX](https://www.bitwig.com/userguide/latest/hardware/) and [hardware integration](https://www.bitwig.com/hardware-integration/).
