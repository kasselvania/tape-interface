# Audio-path findings: firmware 1.1.4 (`e924784c`)

The current stock-monitor control does **not** provide independent local
monitoring and USB capture in the operator's tested setup. Treat that behavior
as a failed requirement for the proposed external-hardware interface, not as a
completed routing feature or a reason to continue the pedal round trip.

## Observed versus required

App: `811c6a6ae9fafa50b85fe35e97967683e32e8ce3`; SDK:
`a9cae67124f4209833e3f63c1cd97b11a8f73070` (API 1.8). The operator reported the
corrected display working, describing approximately 5% input gain. Exact
two-decimal text was not supplied, so this is not an exact 4.95% readback.

With external hardware feeding LINE IN, the operator reported sound at the
speaker and USB when monitor was ON. With monitor OFF, sound disappeared.
On explicit clarification, **Bitwig's input meter/recording also went silent**.
These are operator observations, not collected audio files or measured levels.
No claim of a separate recording artifact, exact buffer size or channel mapping
is made. The DAW configuration remains operator-controlled.

| Path | Required interface behavior | Current evidence |
| --- | --- | --- |
| LINE IN -> local output, monitor ON | Audible | Operator reports audible speaker output |
| LINE IN -> local output, monitor OFF | Silent | Operator reports silence |
| LINE IN -> USB capture, monitor OFF | Input remains available | **FAIL in tested setup:** input meter/recording goes silent |
| USB playback -> physical output, monitor OFF | Playback remains available | **NOT TESTED independently** |
| USB playback -> USB capture with LINE IN disconnected | No unintended return | **NOT TESTED independently** |

The first two rows demonstrate useful monitor control. They do not rescue the
third row: disabling local feed-through must not destroy the pedal-return
capture needed by the intended interface. USB enumeration as 2x2 does not
establish independent routing of the physical inputs and outputs.

## What the available SDK actually establishes

- [Monitor API](../tape_sdk/tapp_api.h): `os_audio_set_monitor()` controls the
  effective live-input monitor. Its playback exception specifically refers to
  samples an app writes to `mixer_get_out()`. It does not explicitly guarantee
  independent USB receive/playback or USB transmit/capture behavior.
- [Recorder example](../tape_sdk/examples/recorder.c): the process callback
  reads the input bus for recording without echoing it. Comments describe mic
  monitoring inside the codec, and line/USB monitoring through the firmware's
  input-to-output addition after the callback. This is evidence for a shared
  monitoring stage, not proof of where USB capture is sourced.
- The input-volume accessor documentation describes one device-wide parameter
  whose callback affects monitoring, recording and USB; it exposes no separate
  USB capture gain or source selector.
- The header deliberately excludes the global mixer routing setters. It also
  exposes no dedicated USB capture-source selector or independent USB playback
  buffer accessor. A search of the available official public organization
  repositories did not locate the installed firmware implementation.
- The [manual](https://b.edti.me/projects/tape/manual/) describes input monitoring
  and class-compliant 2x2 USB audio, but does not specify the USB routing matrix.
  It is marked work in progress and is not a substitute for this hardware result.

## Conclusions and remaining gaps

The live monitor toggle and USB capture are coupled in the observed setup.
That does **not** yet locate the coupling: USB capture might be sourced after
the monitor mix, or disabling monitoring might idle/gate another shared stage.
The installed firmware source and an independently isolated USB playback test
are missing evidence. Do not present either candidate explanation as fact.

C1 controls stock firmware settings only. More UI cannot itself separate
destinations that the exposed controls do not separate. Adding an engine
callback is also not a demonstrated solution: the exposed buses may already
combine physical input and USB playback, and the USB capture tap is unspecified.
The existing no-engine/no-buffer/no-DSP boundary remains in force.

Before selecting another implementation, establish:

1. Where USB capture gets its samples: physical input, input bus, output bus,
   or another mix; and whether monitor OFF changes its source or stream state.
2. Where USB playback enters the firmware chain and whether it remains audible
   with monitor OFF when the physical input is disconnected.
3. Whether any supported firmware setting separates LINE capture, USB playback
   and local monitoring. If not, what firmware/SDK change would expose them.

The next isolated hardware check is USB playback with LINE IN disconnected:
send a known DAW clip to Tape, compare local output with monitor ON and OFF,
and separately observe whether that clip appears on Tape's DAW input with DAW
input monitoring disabled. This fills the two remaining rows without feeding
an analog output back into its input. No new TAPP installation is required.

The assistant's earlier instruction to capture with monitor OFF assumed the
very independence under test. That assumption is withdrawn. Pedal routing
remains blocked by the failed monitor-OFF capture requirement until a supported
route is demonstrated.
