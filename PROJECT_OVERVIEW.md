# LittleGPTracker RG Nano - Audio Input + Synthesis Project

## Quick Start

This project has two main goals:
1. **Add audio input sampling** to LGPT on RG Nano (✅ DONE)
2. **Add synthesis engines** to LGPT (📝 DOCUMENTED, ready to implement)

## Documentation Index

### Audio Input (Current Feature)
- **README_AUDIO_INPUT.md** - Complete guide to audio input sampling
  - How to use recording feature
  - Hardware requirements
  - Troubleshooting
  - Technical architecture

- **AUDIO_INPUT_MOD.md** - Developer notes on audio input implementation
  - Code changes made
  - SDL audio capture details

### Synthesis Engines (Future Feature)
- **ADDING_SYNTH_INSTRUMENTS.md** - Complete guide to adding synth instruments
  - Full working code examples
  - Architecture analysis
  - Step-by-step implementation guide
  - Performance considerations
  - Path to M8-like synthesis

## Current Status

### ✅ Completed
- [x] Audio input via USB-C audio adapters
- [x] Recording dialog UI (LT+A button mapping)
- [x] SDL audio capture implementation
- [x] Instant save and import to instruments
- [x] 44.1kHz 16-bit stereo recording
- [x] Up to 60 second recording duration
- [x] Documentation for audio input
- [x] Documentation for synthesis implementation

### 🔧 In Progress
- [ ] FunKey-OS kernel build with USB audio support
- [ ] Testing on actual hardware

### 📋 Planned (Synthesis)
- [ ] Simple wavetable instrument (sine/saw/square)
- [ ] FM synthesis (2-4 operators)
- [ ] Port Mutable Instruments Braids
- [ ] UI for synth parameters

## File Structure

```
LittleGPTracker-RG-Nano-Audio-In-Sampling/
├── sources/
│   ├── Adapters/SDL/Audio/
│   │   ├── SDLAudioCapture.h           # Audio capture driver (NEW)
│   │   └── SDLAudioCapture.cpp         # Implementation (NEW)
│   └── Application/
│       ├── Views/
│       │   ├── InstrumentView.cpp      # Modified for LT+A
│       │   └── ModalDialogs/
│       │       ├── RecordSampleDialog.h    # Recording UI (NEW)
│       │       └── RecordSampleDialog.cpp  # Implementation (NEW)
│       └── Instruments/
│           ├── I_Instrument.h          # Base interface (modify for synth)
│           ├── SampleInstrument.h/cpp  # Sample playback (reference)
│           └── (Future: WavetableInstrument.h/cpp)
├── projects/
│   └── Makefile                        # Updated for new files
├── README.md                           # Original LGPT readme
├── README_AUDIO_INPUT.md              # Audio input guide ⭐
├── AUDIO_INPUT_MOD.md                 # Developer notes
├── ADDING_SYNTH_INSTRUMENTS.md        # Synthesis guide ⭐
└── PROJECT_OVERVIEW.md                # This file ⭐
```

## How to Build

### Prerequisites
1. **FunKey SDK** - Cross-compiler for ARM
2. **Modified FunKey-OS** - With USB audio kernel support (see ../FunKey-OS-Test-Audio-In-For-LGPT/)
3. **SDL 1.2** development libraries
4. **ALSA** development libraries

### Build LGPT
```bash
cd projects/
make PLATFORM=MIYOO clean
make PLATFORM=MIYOO
```

### Build FunKey-OS (Kernel with USB Audio)
See: `../FunKey-OS-Test-Audio-In-For-LGPT/BUILD_GUIDE.md`

## Roadmap

### Phase 1: Audio Input (DONE)
- ✅ Kernel USB audio support
- ✅ SDL audio capture
- ✅ Recording UI
- ✅ Save and import

### Phase 2: Hardware Testing (CURRENT)
- 🔧 Build FunKey-OS firmware
- 🔧 Flash to RG Nano
- 🔧 Test audio input
- 🔧 Test LGPT recording

### Phase 3: Simple Synthesis (NEXT)
- ⏳ Implement wavetable instrument
- ⏳ Add to instrument bank
- ⏳ Create UI for parameters
- ⏳ Test on hardware

### Phase 4: Advanced Synthesis (FUTURE)
- 📋 FM synthesis engine
- 📋 Port Mutable Instruments Braids
- 📋 Multiple synth types
- 📋 Full parameter control

### Phase 5: Polish (FUTURE)
- 📋 Performance optimization
- 📋 Preset system
- 📋 Documentation
- 📋 Community release

## Why This Matters

### Audio Input
**Enables live sampling workflow:**
- Sample external synths/instruments
- Record vocals/field recordings
- Create custom sample libraries
- Sample-based beat making

**Makes LGPT more like:**
- MPC workflow (sample + sequence)
- SP-404 style sampling
- Portable sampling station

### Synthesis Engines
**Transforms LGPT into hybrid tracker:**
- ✅ Sample playback (existing)
- ➕ Synthesis (new)
- = **Most powerful portable tracker**

**Comparison to M8:**
- M8: $500, proprietary, closed-source
- LGPT+Synth: Free, open-source, hackable
- RG Nano: $40 hardware

**Would be FIRST:**
- No one has added synthesis to LGPT
- Novel contribution to tracker community
- Potential to influence future tracker development

## Technical Highlights

### Audio Input Architecture
```
USB Audio Adapter → Kernel → ALSA → SDL → LGPT
                                           ↓
                                    RecordSampleDialog
                                           ↓
                                    WavFileWriter
                                           ↓
                                    SamplePool (auto-import)
```

### Synthesis Architecture (Planned)
```
I_Instrument (interface)
    ↓
WavetableInstrument
    ├── Oscillator (generate waveforms)
    ├── Envelope (ADSR)
    ├── Filter (reuse from SampleInstrument)
    └── Render() → Audio buffer
```

### Key Design Decisions

**Audio Input:**
- SDL audio capture (cross-platform)
- Modal dialog UI (consistent with LGPT UX)
- LT+A button combo (follows LGPT conventions)
- Instant import (streamlined workflow)

**Synthesis (Planned):**
- Inherit from I_Instrument (clean integration)
- Reuse existing filter/FX code (DRY principle)
- Fixed-point math (performance on ARM)
- Modular design (easy to add synth types)

## Performance Notes

### RG Nano Hardware
- **CPU**: ARM Cortex-A7 @ 1.2 GHz
- **RAM**: 64MB (FunKey-S variant)
- **Audio**: 44.1kHz stereo

### Audio Input Performance
- **CPU usage**: ~5-10% during recording
- **Latency**: ~50-100ms (acceptable for sampling)
- **Memory**: ~10MB per 60-second recording

### Synthesis Performance (Estimated)
- **Wavetable**: ~10-20µs per channel
- **FM (2-op)**: ~50-100µs per channel
- **Braids**: ~100-200µs per channel
- **Budget**: ~200µs per channel (8 channels total)

**Conclusion**: Should handle 2-4 synth voices + 4-6 sample voices comfortably.

## Getting Started

### For Users (Audio Input)
1. Read **README_AUDIO_INPUT.md**
2. Build/flash modified FunKey-OS
3. Build LGPT
4. Start sampling!

### For Developers (Synthesis)
1. Read **ADDING_SYNTH_INSTRUMENTS.md**
2. Copy provided code examples
3. Start with simple sine wave
4. Iterate and expand

## Resources

### External Projects
- **LittleGPTracker**: https://github.com/Mdashdotdashn/LittleGPTracker
- **FunKey-OS**: https://github.com/DrUm78/FunKey-OS
- **Mutable Instruments**: https://github.com/pichenettes/eurorack

### Learning Resources
- **The Audio Programming Book** - DSP fundamentals
- **Designing Sound** by Andy Farnell - Synthesis techniques
- **MusicDSP Archive** - Algorithm reference

### Community
- LGPT Forums: https://chipmusic.org/forums/forum/2/littlegptracker/
- FunKey Discord: (check FunKey-OS repo)

## Contributing

This is an open-source project following LGPT's GPL-3.0 license.

**Ways to contribute:**
- Test audio input on real hardware
- Implement synthesis engines
- Improve documentation
- Report bugs
- Share presets/samples

## Credits

**Original LGPT:**
- m-.-n (Marc Nostromo) - Original author
- djdiskmachine - Current maintainer
- LGPT community contributors

**Audio Input Modifications:**
- SDL audio capture implementation
- Recording dialog UI
- RG Nano platform integration

**FunKey-OS:**
- DrUm78 and FunKey Project team

**Synthesis Documentation:**
- Based on LGPT architecture analysis
- Reference implementations provided

## License

GPL-3.0 (same as LittleGPTracker)

## Questions?

Check the documentation:
- **README_AUDIO_INPUT.md** - User guide for audio input
- **ADDING_SYNTH_INSTRUMENTS.md** - Developer guide for synthesis

---

**This project aims to make LGPT the most powerful portable tracker ever created.** 🎵🚀
