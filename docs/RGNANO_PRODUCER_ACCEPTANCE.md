# RG Nano Producer Acceptance

This document tracks whether the RG Nano fork is credible as a tiny music-production workstation, excluding live sampling. The acceptance target is: a producer can load prepared WAV samples, compose, arrange, edit instruments, preview playback, save/reopen, and understand the current screen on a 240x240 RG Nano display.

## Current Verdict

The core LGPT production spine is present: Song, Chain, Phrase, Instrument, Table, Groove, Project/settings, sample import, sample preview, playback, save/reopen, and simulator capture all exist.

The open question is no longer "can it make a song?" It can. The open question is whether every producer-facing screen is legible enough, whether navigation feels intentional on the RG Nano buttons, and which M8-style features should be added first.

## Screen Audit Matrix

Run `projects/resources/RGNANO_SIM/producer-screen-audit.rgsim` to capture the current view set. The script writes screenshots named `audit-*.bmp` and `dump_state` logs for every screen below.

Latest audit: passed on the native `RGNANO_SIM` executable at 240x240. Every scripted view assertion passed, `expect_no_error` was clean, and visual review confirmed the core tracker screens are readable at RG Nano resolution.

| Screen | Current support | Producer value | Audit status | Notes |
| --- | --- | --- | --- | --- |
| Boot project selector | Existing modal | Load/new project | Confirmed in sim | Must stay readable because every session starts here. |
| New project dialog | Existing modal | Create song folders | Confirmed in sim | Naming UI is cramped but the random-name path is usable. |
| Song view | Existing core view | Arrange chains across 8 tracks | Confirmed in sim | Split mode fits all 8 tracks, but dense rows need continued readability checks. |
| Mixer view | Existing view, route restored | Track activity, play time, CPU/clip/battery | Confirmed in sim | `SongView` could request `VT_MIXER`, but `AppWindow` was not switching to it. This pass restores it and labels the screen as `Mixer Song` or `Mixer Live`. |
| Chain view | Existing core view | Phrase order and transposition | Confirmed in sim | Usable, but transposition readability should be checked at device scale. |
| Phrase view | Existing core view | Notes, instruments, commands | Confirmed in sim | This is the most important editing screen; command columns need special audit. |
| Phrase table | Existing core view | Per-step command automation | Confirmed in sim | Needs command-selector coverage next. |
| Groove view | Existing core view | Swing/timing feel | Confirmed in sim | Good M8-adjacent feature already present. |
| Instrument view | Existing core view | Sample assignment and shaping | Confirmed in sim | Strongest native screen for sound design today, but long labels are cramped. |
| Instrument table | Existing core view | Instrument-local modulation/commands | Confirmed in sim | Important for M8-style movement; needs better discoverability. |
| Sample import modal | Existing modal | Browse, preview, import WAVs | Confirmed in sim | Native `Listen` preview is now assertable. |
| Record modal | Existing code | Live capture | Out of current scope | Leave alone until hardware audio input path is real. |
| Project/settings view | Existing core view | Tempo, scale, render, save/load | Confirmed in sim | Already has global `Scale:` and `Render:` fields, worth elevating in workflow docs. |
| Power menu overlay | RG Nano simulator feature | Exit/debug without leaking input | Confirmed in sim | Text labels now render in screenshots, and the power input remains globally isolated from tracker controls. |
| Command selector/help | Existing modal/helper code | FX command discovery | Needs dedicated route | High priority for usability because M8 has strong command help. |
| Render/export flow | Existing project/render fields | Bounce songs/stems/samples | Needs dedicated test | Project view exposes render mode; simulator captures audio, but producer UX is not audited yet. |

## M8-Inspired Parity Targets

Dirtywave M8 uses the same broad tracker hierarchy: 8 tracks, song rows containing chains, chains containing phrases, and phrases containing notes, instruments, and command effects. Its manual also emphasizes shifted view navigation, a mini-map, shared shortcuts, command help, instrument engines, scales, mixer/effects, render, and sample editing.

Sources used for comparison:

- Dirtywave M8 Model:02 manual PDF: https://images.equipboard.com/uploads/item/manual/136211/dirtywave-m8-tracker-model-02-manual.pdf
- Dirtywave M8 manual page overview: https://www.manualslib.com/manual/3543344/Dirtywave-M8.html
- Dirtywave M8 headless setup docs: https://github.com/Dirtywave/M8Docs/blob/main/docs/M8HeadlessSetup.md
- Dirtywave M8 headless firmware repo: https://github.com/Dirtywave/M8HeadlessFirmware
- m8c headless client: https://m8c.laamaa.fi/

### Keep / Improve From LGPT

| Area | RG Nano fork today | M8 lesson | Proposed direction |
| --- | --- | --- | --- |
| View navigation | R+D-pad graph | M8 uses a consistent shifted directional view map plus mini-map | Add a tiny quick-nav overlay or debug/map screen instead of always-visible mini-map. |
| Song/chain/phrase | Present | M8 makes the hierarchy explicit and fast | Keep LGPT structure; improve screen labels and route hints. |
| Tables/groove | Present | M8 tables/modulation are central to movement | Make table/instrument-table access clearer and test command editing. |
| Project scale | Global `Scale:` exists | M8 has a dedicated Scale View and scale/key workflow | First expose scale workflow better; later add a compact Scale screen. |
| Render | Project render field exists | M8 has a dedicated Render View and render-to-sample workflow | Add a producer-friendly render flow and simulator assertion. |
| Instrument engines | Sample instruments exist | M8 has Wavsynth, Macrosynth, Sampler, FM Synth, Hypersynth, MIDI Out, and External Instrument | Start native and incremental: wavetable/simple synth, then FM or macro-style engines. |

### Add Over Time

| Priority | Feature | Why it matters | Possible sources to study |
| --- | --- | --- | --- |
| P0 | Producer screen audit and screenshots | Stops us from guessing about 240x240 usability | Local simulator |
| P1 | Command help / command selector audit | Essential tracker learning loop on tiny screen | LGPT command selector, M8 command help |
| P1 | Better render/export workflow | Needed to finish tracks on-device | LGPT render code, M8 Render View |
| P1 | Scale workflow | Makes melodic writing much faster | Existing LGPT scale variable, M8 Scale View |
| P2 | Simple synth engines | Reduces dependence on samples | WaveTracker wavetable ideas, picoTracker synth work |
| P2 | Instrument macros/modulation | M8-style movement and sound design | LGPT tables, WaveTracker macros |
| P2 | Mixer/send FX UX | Better finishing inside device | LGPT MixerView, M8 mixer/effects |
| P3 | Sample editor operations | Useful with uploaded samples even without live sampling | M8 sample editor, MilkyTracker/Schism sample tooling |

## Open Source Tracker References

These are not drop-in ports, but they are useful references for feature design and implementation ideas:

- WaveTracker: tracker with wavetable synthesis, sampling, instrument macros, MIDI input, WAV export, oscilloscope, and piano-roll visualizer. https://github.com/squiggythings/WaveTracker
- picoTracker: open-source handheld tracker influenced by LGPT with 8 channels, sample instruments, and synth-instrument experiments. https://github.com/xiphonics/picoTracker
- Schism Tracker: open-source Impulse Tracker-style sample tracker with mature module playback/editing ideas. https://github.com/schismtracker/schismtracker
- MilkyTracker: FastTracker II-compatible open-source tracker useful for sample/instrument UX references. https://github.com/milkytracker/MilkyTracker
- Kunquat: tracker-like sequencer with non-Western scale/tuning goals and flexible instrument/signal processors. https://kunquat.org/
- SunVox: not open-source as an app, but useful as an existence proof for optimized modular synth + tracker workflows on small devices. https://warmplace.ru/soft/sunvox/

## Near-Term Work

1. Run the screen audit after every RG Nano layout change.
2. Review `audit-*.bmp` screenshots at actual 240x240 size, not scaled up.
3. Fix any screen where the selected item, cursor, or active mode is not obvious.
4. Add dedicated tests for command selector/help, render/export, scale editing, and instrument parameter editing.
5. Start M8 parity with native features one at a time: command help, render UX, scale UX, then synth engines.
