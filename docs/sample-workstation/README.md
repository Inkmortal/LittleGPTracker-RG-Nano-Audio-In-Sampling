# RG Nano Sample Workstation Exploration

This branch explores a less text-heavy sample and instrument workflow for the RG Nano. The goal is not to remove tracker power; it is to make sample design feel like a small instrument surface instead of a long settings list.

## Baseline Screens

Current Instrument View:

![Current Instrument View](assets/before-instrument.png)

Current Sample Import modal:

![Current Sample Import](assets/before-sample-import.png)

## Current Problem

The existing sample instrument is already powerful, but the interface hides that power in a dense text stack. Important controls compete for attention:

- sample source
- volume and pan
- root note and detune
- crush, drive, and downsample
- filter cutoff, resonance, type, mode, and attenuation
- feedback tune and mix
- interpolation
- loop mode, slices, start, loop start, and loop end
- automation and instrument table

That is too much for one 240x240 text menu. It is usable for someone who already knows LGPT, but it does not invite playful sound design.

## Implemented Prototype

This branch now replaces the single long sample-instrument list with five Instrument Lab pages. The controls are still the real LGPT sample instrument variables, but each page gets a visual panel sized for the RG Nano screen. Use `L + Left/Right` to switch pages.

### Source

![Instrument Lab Source](assets/instrument-lab-source.png)

Shows the loaded WAV as an actual waveform preview, plus start/loop/end markers and the core source fields.

### Shape

![Instrument Lab Shape](assets/instrument-lab-shape.png)

Groups level, pan, and grit controls so volume/pan/crush/drive/downsample feel like sound-shaping controls instead of isolated rows.

### Filter

![Instrument Lab Filter](assets/instrument-lab-filter.png)

Groups cutoff, resonance, filter type, mode, and attenuation into one tone page.

### Loop

![Instrument Lab Loop](assets/instrument-lab-loop.png)

Uses the same real waveform preview, focused on loop mode, slice count, start, loop start, and loop end.

### Motion

![Instrument Lab Motion](assets/instrument-lab-motion.png)

Puts table automation and feedback controls on a motion page. This is the roughest page visually because the current app has table selection, not a full modulation workstation yet.

## Design Direction

Keep shortcuts fast and memorizable. Use visuals for state, not paragraphs.

Instrument Lab pages:

| Page | Job | Visual Bias |
| --- | --- | --- |
| Source | Pick/import/preview sample, root note, detune, slices | sample name, pitch test, small waveform |
| Shape | Volume, pan, crush, drive, downsample, interpolation | meters, pan marker, grit meter |
| Filter | Cutoff, resonance, filter type/mode, attenuation | filter curve or cutoff bar |
| Loop | Start, loop start, loop end, loop mode, slices | waveform window with loop markers |
| Motion | Instrument table, automation, movement templates | step mini-grid and motion arrows |

## Button Philosophy

- `D-pad`: move focus inside the current page.
- `A + D-pad`: edit the focused value.
- `Start`: audition current instrument in phrase context.
- `R + Start`: audition in song context.
- `R + Left`: return to Phrase.
- `R + Down`: open instrument table/motion.
- A future page-switch shortcut should be easy to chord on the real Nano, likely `L/R + Left/Right` or `Select` only after checking existing mappings.

## First Build Target

The first prototype implements these Instrument Lab pages directly in `InstrumentView`. Use `L + Left/Right` to switch between Source, Shape, Filter, Loop, and Motion. It is still intentionally experimental, but it proves the visual-page direction inside the real app.

The Source/Loop direction matters most because sample-heavy production depends on quickly answering:

- What sample is loaded?
- What part of it plays?
- Is it one-shot, looped, ping-pong, oscillator, or loop-sync?
- What note is it tuned to?
- Does it still sound good when pitched low, normal, and high?

No synth engines yet. Make sample instruments feel great first.

## Current Limitations

- The Source and Loop waveform is read from the loaded sample buffer, so it reflects the actual WAV shape.
- Marker edits still use the normal field workflow below the visual panel.
- Motion is only a visual grouping for existing table/feedback settings; it is not an M8-style modulation matrix yet.
- Sample import is still the existing text list. A visual library/browser is a separate next step.

## Layout Audit

Tiny screens need automated layout checks, not just screenshots. After capturing the Instrument Lab pages with `projects/resources/RGNANO_SIM/sample-lab-pages-preview.rgsim`, run:

```powershell
python ..\tools\rgnano_layout_audit.py .\sample-lab-source.bmp .\sample-lab-shape.bmp .\sample-lab-filter.bmp .\sample-lab-loop.bmp .\sample-lab-motion.bmp
```

The audit fails on:

- screenshots that are not 240x240 app captures
- foreground pixels bleeding onto the app edge
- rows that are too dense to be readable
- Source/Loop waveforms colliding with legend or editable fields
- Shape/Filter/Motion controls pushing into the field list
