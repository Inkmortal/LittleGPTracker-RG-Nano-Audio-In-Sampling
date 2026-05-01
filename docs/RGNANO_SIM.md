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

## Buttons

| RG Nano | Simulator key |
| --- | --- |
| Up | `u` |
| Down | `d` |
| Left | `l` |
| Right | `r` |
| A | `a` |
| B | `b` |
| L | `m` |
| R | `n` |
| Start | `s` |
| Select | `q` |

This target is intended for automated UI, project, sample, and audio-output smoke tests before moving a build to the physical RG Nano.

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

# SDL BMP screenshot
screenshot smoke-song.bmp

# fail unless the SDL surface is exactly 240x240
expect_size 240 240

# fail unless the screen has at least two distinct pixel values
expect_colors 2

quit
```

Screenshots and assertions are captured from the SDL surface, so they are available to automated tests without a camera or physical RG Nano.

The canonical smoke test is:

```powershell
.\run.ps1 -Task smoke
```
