# RG Nano Producer Acceptance

This document tracks whether the RG Nano fork is credible as a tiny music-production workstation, excluding live sampling. The acceptance target is: a producer can load prepared WAV samples, compose, arrange, edit instruments, preview playback, save/reopen, and understand the current screen on a 240x240 RG Nano display.

The broader M8 parity audit lives in [RGNANO_M8_SCREEN_CAPABILITY_AUDIT.md](RGNANO_M8_SCREEN_CAPABILITY_AUDIT.md). Use that file for the durable list of implemented, partial, missing, and audit-needed screens/capabilities.

## Current Verdict

The core LGPT production spine is present: Song, Chain, Phrase, Instrument, Table, Groove, Project/settings, sample import, sample preview, playback, save/reopen, all 8 tracker channels, and simulator capture all exist.

The open question is no longer "can it make a song?" It can. The open question is whether every producer-facing screen is legible enough, whether navigation feels intentional on the RG Nano buttons, and which M8-style features should be added first.

## Screen Audit Matrix

Run the full app-only layout audit with:

```powershell
python tools\run_producer_layout_audit.py
```

That runner builds `RGNANO_SIM`, runs `projects/resources/RGNANO_SIM/producer-screen-audit-appshots.rgsim`, captures every core producer-facing screen as `audit-*.bmp`, applies the programmatic 240x240 layout audit, and cleans generated files afterward.

For manual inspection with state logs, `projects/resources/RGNANO_SIM/producer-screen-audit.rgsim` still captures the current view set and writes `dump_state` logs for every screen below.

Latest audit: passed on the native `RGNANO_SIM` executable at 240x240. Every scripted view assertion passed, `expect_no_error` was clean, and visual review confirmed the core tracker screens are readable at RG Nano resolution.

| Screen | Current support | Producer value | Audit status | Notes |
| --- | --- | --- | --- | --- |
| Boot project selector | Existing modal | Load/new project | Confirmed in sim | Must stay readable because every session starts here. |
| New project dialog | Existing modal | Create song folders | Confirmed in sim | Naming UI is cramped but the random-name path is usable. |
| Song view | Existing core view | Arrange chains across 8 tracks | Confirmed in sim | Split mode fits all 8 tracks, but dense rows need continued readability checks. |
| 8-channel playback | Existing core engine plus live Nano meters | Full arrangements across all tracks | Confirmed in sim | `all-8-channels-workflow.rgsim` assigns independent chains, phrases, notes, and instruments to channels 0-7, asserts all 8 channels are playing, captures audio, and screenshots the active note/instrument monitor plus one-row meter strip. |
| Mixer view | Existing view, route restored, plus live master waveform | Track activity, play time, CPU/clip/battery, output shape | Confirmed in sim | `SongView` could request `VT_MIXER`, but `AppWindow` was not switching to it. This pass restores it, labels the screen as `Mixer Song` or `Mixer Live`, and `mixer-waveform-workflow.rgsim` verifies the live waveform during playback. |
| Context/helper overlay | Universal `R + Select` overlay on every visible LGPT screen | Fast orientation on a screen with no room for permanent help | Confirmed in sim | Page 1 shows current screen name, the Song -> Chain -> Phrase minimap, main exits, and primary edit gesture. `Up`/`Down` switches to a command page with inputs for the current screen. It blocks normal inputs while open and closes with `R + Select`, leaving raw `Select` free for focused producer actions. On real hardware, Select is the OS `FN` key (`KEY_K`). |
| Chain view | Existing core view plus dynamic sidebar meters | Phrase order, transposition, channel activity | Confirmed in sim | Usable, and the right sidebar meter now reflects live audio peak/activity instead of a static playing marker. |
| Phrase view | Existing core view plus sharp/flat note display, key-aware scale stepping, and chromatic override | Notes, instruments, commands | Confirmed in sim | This is the most important editing screen; command columns need special audit. `note-spelling-workflow.rgsim` verifies `C#3`/`Db3` display switching, `scale-key-workflow.rgsim` verifies keyed scale note edits through `A + D-pad`, and `scale-free-override-workflow.rgsim` verifies free default editing plus `L + D-pad` outside notes. |
| Phrase table | Existing core view | Per-step command automation | Confirmed in sim | `command-selector-workflow.rgsim` verifies `Select` opens the command selector from Table command columns. |
| Groove view | Existing core view | Swing/timing feel | Confirmed in sim | Good M8-adjacent feature already present. |
| Instrument view | Existing core view | Sample assignment and shaping | Confirmed in sim | Strongest native screen for sound design today, but long labels are cramped. |
| Instrument table | Existing core view | Instrument-local modulation/commands | Confirmed in sim | Important for M8-style movement; needs better discoverability. |
| Sample import modal | Existing modal | Browse, preview, import WAVs | Confirmed in sim | Native `Listen` preview is now assertable. |
| Record modal | Existing code | Live capture | Out of current scope | Leave alone until hardware audio input path is real. |
| Project/settings view | Existing core view plus musical context controls | Tempo, key, scale, note spelling, render, save/load | Confirmed in sim | New projects show `Key: --` by default for normal chromatic editing. Setting `Key:` enables scale-guided note edits; it does not rewrite existing notes, and `L + D-pad` provides a one-hand chromatic override. |
| Power menu overlay | RG Nano simulator feature only | Exit/debug test harness without leaking input | Confirmed in sim | Text labels render in screenshots, and the simulator power input remains isolated from tracker controls. Real RG Nano hardware keeps Menu/Power under OS control for volume, brightness, and system actions. |
| Command selector/help | Existing modal/helper code plus raw `Select` shortcut on command columns | FX command discovery | Confirmed in sim | `command-selector-workflow.rgsim` verifies Phrase command selection, command persistence, parameter editing, and Table command selector access. The selector now draws help below the command grid instead of overwriting it. |
| Runtime command behavior | `TABL`, `GROV`, and `TMPO` verified through player/model hooks | Arrangement movement and song-level changes | Confirmed in sim | `command-runtime-workflow.rgsim` verifies `TABL` starts table playback, `GROV` changes channel groove state, and `TMPO` changes tempo during playback. Audio-shaping commands such as `VOLM`, `PTCH`, `RTRG`, `PLOF`, and `LEGA` still need audio-measurement assertions. |
| Playback scope visibility | Tiny `PLAY:` label on every main view | Know whether Start is playing song, chain, phrase, live, or audition scope | Confirmed in sim | `playback-scope-workflow.rgsim` verifies Song `PLAY:SONG`, Chain `PLAY:CHAIN`, Phrase/Table `PLAY:PHR`, and Phrase `R + Start` as song-context playback. |
| Render/export flow | Existing project/render fields plus native WAV assertions | Bounce songs and stems | Confirmed in sim | `render-export-workflow.rgsim` sets Project `Render: Stereo`, verifies `mixdown.wav`, then sets `Render: Stems` and verifies `channel0.wav`. Dedicated M8-style render-to-sample and range/name controls are still missing. |

## M8-Inspired Parity Targets

Dirtywave M8 uses the same broad tracker hierarchy: 8 tracks, song rows containing chains, chains containing phrases, and phrases containing notes, instruments, and command effects. Its manual also emphasizes shifted view navigation, a mini-map, shared shortcuts, command help, instrument engines, scales, mixer/effects, render, and sample editing.

Sources used for comparison:

- RG Nano parity map: [M8_WORKFLOW_MAP.md](M8_WORKFLOW_MAP.md)
- Dirtywave official resources/downloads page: https://dirtywave.com/pages/resources-downloads
- Dirtywave M8 operation manual PDF, Version 6.5.2, 2026-04-21: https://cdn.shopify.com/s/files/1/0455/0485/6229/files/m8_operation_manual_v20260421.pdf?v=1776791699
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
| Project scale/key | Global `Key:` and `Scale:` exist | M8 has a dedicated Scale View and scale/key workflow | Keep the project-level controls as the tiny-screen default. Future thought: M8-style parity should probably use a phrase/table command such as `SCA`/`KEY` for per-track or time-position scale/key changes, not a separate song-row key map. |
| Note spelling | Project `Notes:` setting supports sharps/flats | M8 keeps musical context visible while editing | Keep this as a simple Project setting; later add automatic spelling from key/scale. |
| Render | Project render field exports `mixdown.wav` and per-channel stem WAVs | M8 has a dedicated Render View and render-to-sample workflow | Keep the proven Project flow, then add clearer output naming/range controls and render-to-sample parity. |
| Instrument engines | Sample instruments exist | M8 has Wavsynth, Macrosynth, Sampler, FM Synth, Hypersynth, MIDI Out, and External Instrument | Start native and incremental: wavetable/simple synth, then FM or macro-style engines. |

### Add Over Time

| Priority | Feature | Why it matters | Possible sources to study |
| --- | --- | --- | --- |
| P0 | Producer screen audit and screenshots | Stops us from guessing about 240x240 usability | Local simulator |
| P1 | Command help / command selector audit | Essential tracker learning loop on tiny screen | LGPT command selector, M8 command help |
| P1 | Better render/export workflow | Native stereo/stems render is proven; producers still need clearer naming/range/render-to-sample UX | LGPT render code, M8 Render View |
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

1. Run `python tools\run_producer_layout_audit.py` after every RG Nano layout change.
2. Review `audit-*.bmp` screenshots at actual 240x240 size, not scaled up.
3. Fix any screen where the selected item, cursor, or active mode is not obvious.
4. Add dedicated tests for instrument parameter editing and deeper audio-shaping commands.
5. Start M8 parity with native features one at a time: render-to-sample/range UX, scale UX, then synth engines.
