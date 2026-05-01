# LittleGPTracker RG Nano Audio-In/Sampling

![Piggy](https://avatars.githubusercontent.com/u/180156201?s=400&u=ebb53bdea61a025edce0c3782ac75b532dd65dd7&v=4)

This is an experimental **LittleGPTracker / Little Piggy Tracker** fork focused on making the **Anbernic RG Nano** useful as a pocket-sized music workstation.

The immediate goal is not to clone Dirtywave M8 firmware. It is to build a native RG Nano tracker workflow that feels serious on the actual device: compose with LGPT-style tracker sequencing, import and manage samples, refine the 240x240 interface, and test changes through a desktop simulator before copying builds to hardware.

The longer-term target is a tiny on-the-go production setup where the RG Nano can act as the final arrangement and production box for sketches made on small devices such as a PO-33, chord/synth gadgets, or prepared sample packs.

## Current Focus

- native RG Nano builds and OPK packaging
- RG Nano-friendly project, song, instrument, and sample workflows
- audio input / sampling experiments where the hardware and OS allow it
- sample upload/import workflow for cases where live recording is not available
- 240x240 UI refinements for the RG Nano screen
- desktop `RGNANO_SIM` build for scripted smoke tests and screenshot capture

## Current Status

- Public experimental fork, not a polished release.
- RG Nano-specific code exists for packaging, UI layout, audio capture experiments, and simulator support.
- The simulator can build on Windows with MSYS2/MinGW, run scripted input, write logs, and capture SDL screenshots.
- Hardware testing is still required before treating any build as performance-ready.

For the simulator, see [docs/RGNANO_SIM.md](docs/RGNANO_SIM.md).

## Quick Start

Build the simulator:

```powershell
.\run.ps1 -Task dev -NoSetup
```

Run the simulator smoke test:

```powershell
.\run.ps1 -Task smoke -NoSetup
```

The smoke test boots the RG Nano simulator, waits for the project screen, captures `smoke-boot.bmp`, and exits.

## About This Fork

This repository is a fork of [djdiskmachine/LittleGPTracker](https://github.com/djdiskmachine/LittleGPTracker), which is itself derived from the original work by `m-.-n`, aka [Marc Nostromo](https://github.com/Mdashdotdashn/LittleGPTracker).

That upstream work made this RG Nano experiment possible. This fork keeps the upstream license notices and adds project-specific RG Nano changes on top.

## License

See [LICENSE](LICENSE). In short:

- the original LittleGPTracker lineage includes BSD 3-Clause terms
- this project carries GPLv3 terms for the current fork
- this is free/open-source software, so forks, experiments, and redistribution are welcome under the actual license terms

Back up your tracker projects and samples before testing experimental builds on hardware.

## Releases

Latest upstream Little Piggy Tracker releases:

- [djdiskmachine/LittleGPTracker releases](https://github.com/djdiskmachine/LittleGPTracker/releases)

RG Nano builds from this fork are experimental. Check this repository's releases or build from source when no release is available.

Archived 2006 builds:

- [Stable & Ghetto](https://github.com/NinjasCL-archive/LittleGPTracker/releases/tag/v1)

## Documentation

General Little Piggy Tracker documentation lives in [docs](docs).

Recommended reading:

- [What is Little Piggy Tracker](docs/wiki/What-is-LittlePiggyTracker.md)
- [Quick-Start Guide](docs/wiki/quick_start_guide.md)
- [Little Piggy Tracker Configuration](docs/LittlePiggyTrackerConf.md)
- [Tips and Tricks](docs/wiki/tips_and_tricks.md)
- [RG Nano Simulator](docs/RGNANO_SIM.md)

## Platform Notes

This fork inherits upstream platform support for desktop and handheld builds. The RG Nano-specific work is experimental and may move faster than upstream release branches.

The upstream tracker also supports platforms such as Windows, macOS, Linux, PSP, Miyoo Mini, RG35XX Plus, and other handheld targets. MIDI, SoundFont, and PrintFX support vary by platform and depend on the OS/audio stack.
