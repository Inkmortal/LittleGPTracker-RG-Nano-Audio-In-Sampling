# RG Nano Simulator

`RGNANO_SIM` is a desktop SDL build that keeps the RG Nano app contract:

- 240x240 logical screen
- 240x240 SDL surface by default, matching the RG Nano pixel dimensions
- RG Nano button names and compact key layout
- desktop audio output through the Windows SDL/W32 backend
- event logging through `DUMPEVENT`
- normal LGPT project/sample folders under `rgnano-sim-data`

## Build

```powershell
.\run.ps1 -Task dev
```

The current workspace uses MSYS2 MinGW and the repo's bundled SDL1 library for this target.

## Run

Copy `projects/resources/RGNANO_SIM/config.xml` and `mapping.xml` next to the simulator executable, then run:

```powershell
.\lgpt-rgnano-sim.exe
```

Or use the helper:

```powershell
.\tools\run-rgnano-sim.ps1
```

Running without a script opens the RG Nano skin by default. The skin has a 240x240 tracker screen, a colored RG Nano-inspired body, clickable simulated controls, top shoulder buttons, USB-C detail, and side power/SD-card hints. Scripted smoke tests keep the exact bare 240x240 SDL surface unless you pass `-Skin`.

```powershell
.\tools\run-rgnano-sim.ps1 -Skin
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\smoke.rgsim
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\smoke.rgsim -Skin
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\sample-fixture.rgsim -Skin -SeedSampleFixture -ArtifactsDir .\sim-artifacts
```

## Buttons

| RG Nano | Simulator key |
| --- | --- |
| Up | `u` |
| Down | `d` |
| Left | `l` |
| Right | `r` |
| A | `a` |
| B | `b` |
| X | `x` |
| Y | `y` |
| L | `m` |
| R | `n` |
| Start | `s` |
| Select | `q` |

In skin mode, these controls can also be clicked directly on the device shell.

This target is intended for automated UI, project, sample, and audio-output smoke tests before moving a build to the physical RG Nano.

The source-derived control and navigation map lives in [RGNANO_INPUT_MAP.md](RGNANO_INPUT_MAP.md). Use that map as the authority for simulator workflows: LGPT handles held button masks, so tests should use real combos instead of treating buttons as isolated clicks.

## Scripted Smoke Tests

Pass a script with:

```powershell
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\smoke.rgsim
```

Script commands:

```text
# milliseconds
wait 500

# key duration-ms
press a 80
press n 80
press r 80

# hold/release
down m
press q 80
up m

# click x y duration-ms
click 250 372 100

# SDL BMP screenshot
screenshot smoke-song.bmp

# fail unless a file exists
expect_file rgnano-sim-data/samples/rgnano-test-tone.wav

# fail unless the simulator log contains text
expect_log Loaded

# fail unless the active LGPT view matches the source-derived view name
expect_view song

# fail if the simulator log contains an error marker
expect_no_error

# fail unless the SDL surface is exactly 240x240
expect_size 240 240

# fail unless the screen has at least two distinct pixel values
expect_colors 2

quit
```

Screenshots and assertions are captured from the SDL surface, so they are available to automated tests without a camera or physical RG Nano.

Scripts can also use source-derived route helpers. The PowerShell runner expands these before launching the simulator, so the C++ simulator still receives ordinary button events:

```text
route boot.new_project_random
route project.to_song
route song.to_chain
route chain.to_phrase
route phrase.to_instrument
route instrument.open_sample_import
```

Route helpers live in `tools\rgnano-sim-routes.ps1`. They are intentionally small and explicit, so routes like `phrase.to_table` remain tied to the source map instead of hidden in test-specific scripts.

The canonical smoke test is:

```powershell
.\run.ps1 -Task smoke
```

The sample fixture smoke test generates a tiny 440 Hz WAV file in `rgnano-sim-data/samples`, verifies it exists, checks the simulator log, exercises a skinned button click, captures a screenshot, and copies logs/screenshots into `sim-artifacts`:

```powershell
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\sample-fixture.rgsim -Skin -SeedSampleFixture -ArtifactsDir .\sim-artifacts
```

The source-derived new-project route drives the actual boot controls from `SelectProjectDialog` and `NewProjectDialog`:

```powershell
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\new-project-route.rgsim -ResetLastProject -ArtifactsDir .\sim-artifacts
```

The first source-derived music workflow creates a new project, creates a chain, creates a phrase, creates the first note/instrument, opens Instrument, then opens the sample import modal:

```powershell
.\tools\run-rgnano-sim.ps1 -Script .\projects\resources\RGNANO_SIM\basic-music-workflow.rgsim -ResetLastProject -ArtifactsDir .\sim-artifacts
```
