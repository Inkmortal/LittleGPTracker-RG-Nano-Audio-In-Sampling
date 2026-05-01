# LittleGPTracker RG Nano Audio-In/Sampling Fork

![Piggy](https://github.com/user-attachments/assets/827fc87f-7751-48ae-9de1-60a5d9a3b5c2)

This repository is an experimental RG Nano-focused fork of **Little Piggy Tracker** / **LittleGPTracker**, a music tracker optimized for portable game consoles.

The goal of this fork is to turn the RG Nano into a tiny, practical music-production device: tracker sequencing, sample-based composition, RG Nano UI/audio improvements, and a desktop simulator so changes can be tested before copying builds to hardware.

## Current Focus

- RG Nano builds and packaging
- audio input / sampling workflow experiments
- sample import and project workflow improvements
- 240x240 tracker UI refinements
- RG Nano simulator build for scripted desktop smoke tests

For the simulator, see [docs/RGNANO_SIM.md](docs/RGNANO_SIM.md).

## About This Fork

This repository is a fork of [djdiskmachine/LittleGPTracker](https://github.com/djdiskmachine/LittleGPTracker), which is itself derived from the original work by `m-.-n`, aka [Marc Nostromo](https://github.com/Mdashdotdashn/LittleGPTracker).

That upstream work made this RG Nano experiment possible. This fork keeps the upstream license notices and adds project-specific RG Nano changes on top.

## Public Repository Policy

This repository is public so users can inspect the source, build the RG Nano version, follow development, and preserve the legal freedoms granted by the upstream licenses.

This particular fork is maintainer-directed. Issues are disabled, and unsolicited pull requests are not the preferred way to propose changes. If you want to experiment heavily, please do it in your own fork, as permitted by the license. Changes may or may not be accepted back here.

No extra trademark, endorsement, warranty, or support rights are granted by this fork.

## License

See [LICENSE](LICENSE). In short:

- the original LittleGPTracker lineage includes BSD 3-Clause terms
- this project carries GPLv3 terms for the current fork
- because this is free/open-source software, public redistribution and forks cannot be prohibited beyond the actual license terms

All implemented features have been tested not to break old
projects but make sure to backup your old cherished work
just to be safe &#9829;

## Releases

### Upstream Builds

Latest upstream Little Piggy Tracker releases:

- [Releases](https://github.com/djdiskmachine/LittleGPTracker/releases)

RG Nano builds from this fork are experimental. Check this repository's releases or build from source when no release is available.

### 2006 Builds

Fetch archived OG builds here:

- [Stable & Ghetto](https://github.com/NinjasCL-archive/LittleGPTracker/releases/tag/v1)

## Community

Join our [Discord server](https://discord.gg/e4N2VM7sz6)

## Documentation

All the relevant documentation can be found in [Docs](docs) directory.

Recommended reading to get you started:

- [What is Little Piggy Tracker](docs/wiki/What-is-LittlePiggyTracker.md)
- [Quick-Start Guide](docs/wiki/quick_start_guide.md)
- [Little Piggy Tracker Configuration](docs/LittlePiggyTrackerConf.md)
- [Tips and Tricks](docs/wiki/tips_and_tricks.md)
- [RG Nano Simulator](docs/RGNANO_SIM.md)

## Features per platform

| Platform    | MIDI_Possible | MIDI_enabled | Soundfonts | Note                                 |
|-------------|---------------|--------------|------------|--------------------------------------|
| PSP         | NO            | NO           | YES        | [See notes](projects/resources/PSP/INSTALL_HOW_TO.txt) |
| DEB         | YES           | YES          | YES        |                                      |
| X64         | YES           | YES          | NO         |                                      |
| X86         | YES           | YES          | YES        |                                      |
| STEAM       | YES           | YES          | NO         |                                      |
| MIYOO       | NO            | NO           | YES        | Port by [Nine-H](https://ninethehacker.xyz) |
| W32         | YES           | YES          | YES        | Built in VS2008 with love            |
| RASPI       | YES           | YES          | YES        | Versatile platform                   |
| CHIP        | YES           | YES          | YES        | [See notes](projects/resources/CHIP/INSTALL_HOW_TO.txt) |
| BITTBOY     | MAYBE         | NO           | YES        |                                      |
| GARLIC      | MAYBE         | NO           | YES        | Port by [Simotek](http://simotek.net)|
| GARLICPLUS  | MAYBE         | NO           | YES        | Port by [Simotek](http://simotek.net)|
| RG35XXPLUS  | MAYBE         | NO           | YES        | Port by [Simotek](http://simotek.net)|
| MACOS       | YES           | YES          | NO         | Port by [clsource](https://genserver.social/clsource) |


* **Soundfont library is currently not ported for 64bit OS**
* **MIDI functionality __greatly__ depends on kernel support, please feature request your favourite OS maintainer =)**
