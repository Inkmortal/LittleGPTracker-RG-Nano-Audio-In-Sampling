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

## Design Direction

Keep shortcuts fast and memorizable. Use visuals for state, not paragraphs.

Proposed Instrument Lab pages:

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

Start with a visual Source/Loop page, because sample-heavy production depends on quickly answering:

- What sample is loaded?
- What part of it plays?
- Is it one-shot, looped, ping-pong, oscillator, or loop-sync?
- What note is it tuned to?
- Does it still sound good when pitched low, normal, and high?

No synth engines yet. Make sample instruments feel great first.
