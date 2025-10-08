# Adding Synthesis Instruments to LittleGPTracker

## Overview

LittleGPTracker (LGPT) has a clean, extensible instrument architecture that makes adding synthesis engines straightforward. This guide explains how to add synth instruments alongside the existing sample-based instruments.

## Architecture Analysis

### Current Instrument System

LGPT uses an abstract interface pattern for instruments:

```
I_Instrument (abstract interface)
    ├── SampleInstrument (plays samples)
    └── MidiInstrument (MIDI output)
```

**Key files:**
- `sources/Application/Instruments/I_Instrument.h` - Abstract interface
- `sources/Application/Instruments/SampleInstrument.h/cpp` - Sample playback
- `sources/Application/Instruments/InstrumentBank.h/cpp` - Factory/manager
- `sources/Application/Mixer/` - Audio mixing system

### The I_Instrument Interface

Every instrument must implement:

```cpp
class I_Instrument {
public:
    // Initialize the instrument
    virtual bool Init() = 0;

    // Start/stop note playback
    virtual bool Start(int channel, unsigned char note, bool retrigger=true) = 0;
    virtual void Stop(int channel) = 0;

    // CRITICAL: Audio generation happens here
    virtual bool Render(int channel, fixed *buffer, int size, bool updateTick) = 0;

    // Process effect commands (volume, filter, etc)
    virtual void ProcessCommand(int channel, FourCC cc, ushort value) = 0;

    // Other required methods...
    virtual InstrumentType GetType() = 0;
    virtual const char *GetName() = 0;
};
```

**The `Render()` method** is where audio generation happens:
- `channel` - Which of 8 channels is playing
- `buffer` - Output buffer (interleaved stereo, fixed-point format)
- `size` - Number of stereo samples to generate
- Returns: `true` if generated audio, `false` if silent

### Instrument Types

Currently defined in `I_Instrument.h`:

```cpp
enum InstrumentType {
    IT_SAMPLE = 0,
    IT_MIDI,
    IT_LAST  // <- Add new types before this
};
```

## Adding a Synth Instrument: Step-by-Step Guide

### Phase 1: Simple Wavetable Oscillator (Weekend Project)

#### 1. Create WavetableInstrument.h

**Location:** `sources/Application/Instruments/WavetableInstrument.h`

```cpp
#ifndef _WAVETABLE_INSTRUMENT_H_
#define _WAVETABLE_INSTRUMENT_H_

#include "I_Instrument.h"
#include "Foundation/Types/Types.h"
#include "Application/Utils/fixed.h"

#define WAVETABLE_SIZE 256
#define WAVETABLE_COUNT 8

class WavetableInstrument : public I_Instrument {
public:
    WavetableInstrument();
    virtual ~WavetableInstrument();

    // I_Instrument interface
    virtual bool Init();
    virtual bool Start(int channel, unsigned char note, bool retrigger=true);
    virtual void Stop(int channel);
    virtual bool Render(int channel, fixed *buffer, int size, bool updateTick);
    virtual void OnStart();
    virtual bool IsInitialized();
    virtual bool IsEmpty();
    virtual InstrumentType GetType() { return IT_SYNTH; }
    virtual const char *GetName();
    virtual void ProcessCommand(int channel, FourCC cc, ushort value);
    virtual void Purge();
    virtual int GetTable();
    virtual bool GetTableAutomation();
    virtual void GetTableState(TableSaveState &state);
    virtual void SetTableState(TableSaveState &state);

private:
    // Per-channel state
    struct ChannelState {
        bool active;
        unsigned char note;
        fixed phase;        // Current position in waveform (0.0-1.0)
        fixed phaseStep;    // How much to advance per sample
        int volume;         // 0-255
        int waveform;       // Which wavetable (0-7)
    };

    ChannelState channels_[8];  // 8 channels max

    // Wavetable storage
    short wavetables_[WAVETABLE_COUNT][WAVETABLE_SIZE];

    // Helper methods
    void initializeWavetables();
    fixed noteToFrequency(unsigned char note);
    short renderSample(int channel);
};

#endif
```

#### 2. Create WavetableInstrument.cpp

**Location:** `sources/Application/Instruments/WavetableInstrument.cpp`

```cpp
#include "WavetableInstrument.h"
#include <math.h>

#define SAMPLE_RATE 44100.0f

WavetableInstrument::WavetableInstrument() {
    for (int i = 0; i < 8; i++) {
        channels_[i].active = false;
        channels_[i].phase = 0;
        channels_[i].volume = 200;  // Default volume
        channels_[i].waveform = 0;
    }
}

WavetableInstrument::~WavetableInstrument() {
}

bool WavetableInstrument::Init() {
    initializeWavetables();
    return true;
}

void WavetableInstrument::initializeWavetables() {
    // Wavetable 0: Sine wave
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        float phase = (float)i / WAVETABLE_SIZE * 2.0f * M_PI;
        wavetables_[0][i] = (short)(sin(phase) * 32767.0f);
    }

    // Wavetable 1: Square wave
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        wavetables_[1][i] = (i < WAVETABLE_SIZE/2) ? 32767 : -32767;
    }

    // Wavetable 2: Sawtooth
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        wavetables_[2][i] = (short)(((float)i / WAVETABLE_SIZE * 2.0f - 1.0f) * 32767.0f);
    }

    // Wavetable 3: Triangle
    for (int i = 0; i < WAVETABLE_SIZE; i++) {
        float t = (float)i / WAVETABLE_SIZE;
        float value = (t < 0.5f) ? (t * 4.0f - 1.0f) : (3.0f - t * 4.0f);
        wavetables_[3][i] = (short)(value * 32767.0f);
    }

    // Initialize remaining wavetables (pulse variations, etc)
    // ...
}

fixed WavetableInstrument::noteToFrequency(unsigned char note) {
    // MIDI note to frequency conversion
    // A4 (note 69) = 440 Hz
    // Formula: freq = 440 * 2^((note - 69) / 12)
    float freq = 440.0f * powf(2.0f, (note - 69) / 12.0f);
    return i2fp((int)freq);
}

bool WavetableInstrument::Start(int channel, unsigned char note, bool retrigger) {
    if (channel < 0 || channel >= 8) return false;

    channels_[channel].active = true;
    channels_[channel].note = note;

    if (retrigger) {
        channels_[channel].phase = 0;
    }

    // Calculate phase increment
    fixed freq = noteToFrequency(note);
    channels_[channel].phaseStep = fp_div(freq, i2fp(SAMPLE_RATE));

    return true;
}

void WavetableInstrument::Stop(int channel) {
    if (channel >= 0 && channel < 8) {
        channels_[channel].active = false;
    }
}

short WavetableInstrument::renderSample(int channel) {
    ChannelState &ch = channels_[channel];

    // Get wavetable position
    int tablePos = fp2i(fp_mul(ch.phase, i2fp(WAVETABLE_SIZE))) % WAVETABLE_SIZE;

    // Read from wavetable
    short sample = wavetables_[ch.waveform][tablePos];

    // Apply volume
    sample = (sample * ch.volume) >> 8;

    // Advance phase
    ch.phase = fp_add(ch.phase, ch.phaseStep);
    if (ch.phase >= i2fp(1)) {
        ch.phase = fp_sub(ch.phase, i2fp(1));
    }

    return sample;
}

bool WavetableInstrument::Render(int channel, fixed *buffer, int size, bool updateTick) {
    if (channel < 0 || channel >= 8) return false;
    if (!channels_[channel].active) return false;

    // Generate 'size' stereo samples
    for (int i = 0; i < size; i++) {
        short sample = renderSample(channel);

        // Convert to fixed point and write to stereo buffer
        buffer[i * 2] = i2fp(sample);      // Left channel
        buffer[i * 2 + 1] = i2fp(sample);  // Right channel
    }

    return true;
}

void WavetableInstrument::ProcessCommand(int channel, FourCC cc, ushort value) {
    if (channel < 0 || channel >= 8) return;

    // Handle commands (volume, waveform select, etc)
    // Example: 'VOLM' command for volume
    if (cc == MAKE_FOURCC('V','O','L','M')) {
        channels_[channel].volume = value & 0xFF;
    }
    // Add more command handlers...
}

bool WavetableInstrument::IsInitialized() { return true; }
bool WavetableInstrument::IsEmpty() { return false; }
const char* WavetableInstrument::GetName() { return "SYNTH"; }
void WavetableInstrument::OnStart() {}
void WavetableInstrument::Purge() {}
int WavetableInstrument::GetTable() { return 0; }
bool WavetableInstrument::GetTableAutomation() { return false; }
void WavetableInstrument::GetTableState(TableSaveState &state) {}
void WavetableInstrument::SetTableState(TableSaveState &state) {}
```

#### 3. Update InstrumentType enum

**File:** `sources/Application/Instruments/I_Instrument.h`

```cpp
enum InstrumentType {
    IT_SAMPLE = 0,
    IT_MIDI,
    IT_SYNTH,    // <- ADD THIS
    IT_LAST
};
```

#### 4. Add to InstrumentBank

**File:** `sources/Application/Instruments/InstrumentBank.cpp`

Find the instrument creation code and add:

```cpp
#include "WavetableInstrument.h"

// In the factory/creation method:
I_Instrument* InstrumentBank::CreateInstrument(InstrumentType type) {
    switch(type) {
        case IT_SAMPLE:
            return new SampleInstrument();
        case IT_MIDI:
            return new MidiInstrument();
        case IT_SYNTH:
            return new WavetableInstrument();  // <- ADD THIS
        default:
            return NULL;
    }
}
```

#### 5. Update Build System

**File:** `projects/Makefile` (or appropriate makefile)

Add to source files list:

```makefile
SOURCES += \
    sources/Application/Instruments/WavetableInstrument.cpp
```

### Phase 2: Add Parameters and UI (1-2 Weeks)

#### Add Synth-Specific Parameters

Extend `WavetableInstrument` with variables (like `SampleInstrument` does):

```cpp
class WavetableInstrument : public I_Instrument {
private:
    // Use LGPT's Variable system for UI integration
    Variable *volume_;
    Variable *waveform_;      // Waveform selector (0-7)
    Variable *detune_;        // Fine pitch adjustment
    Variable *attack_;        // Envelope attack
    Variable *decay_;         // Envelope decay
    Variable *sustain_;       // Envelope sustain
    Variable *release_;       // Envelope release
};
```

#### Create Instrument View

Follow the pattern in `InstrumentView.cpp` to add synth parameter editing screens.

### Phase 3: Advanced Features (1+ Month)

#### Add FM Synthesis

Create `FMInstrument.h/cpp` with 2-4 operators:

```cpp
class FMInstrument : public I_Instrument {
private:
    struct Operator {
        fixed phase;
        fixed phaseStep;
        int level;        // Operator volume
        int feedback;     // Self-modulation
    };

    Operator operators_[4];  // 4 operators
    int algorithm_;          // Routing (0-11 like DX7)
};
```

#### Port Mutable Instruments Braids

Braids is open-source (MIT license) and ARM-optimized:

**Repository:** `https://github.com/pichenettes/eurorack/tree/master/braids`

**Integration steps:**
1. Copy `braids/` source to `sources/Externals/braids/`
2. Create `BraidsInstrument.h/cpp` wrapper
3. Hook Braids' `Render()` into LGPT's `Render()`
4. Map LGPT parameters to Braids' macro controls

**Braids provides:**
- 44 synthesis algorithms
- Wavetable, FM, physical modeling, drums
- Optimized for ARM Cortex-M (will run on Cortex-A7)

## Key Concepts

### Fixed-Point Math

LGPT uses fixed-point instead of floating-point for performance:

```cpp
#include "Application/Utils/fixed.h"

// Conversions
fixed value = i2fp(100);      // Integer to fixed
int result = fp2i(value);     // Fixed to integer

// Math operations
fixed sum = fp_add(a, b);     // Addition
fixed product = fp_mul(a, b); // Multiplication
fixed quotient = fp_div(a, b);// Division
```

### Audio Format

LGPT uses:
- **Sample rate:** 44100 Hz
- **Format:** Interleaved stereo, fixed-point
- **Buffer layout:** `[L0, R0, L1, R1, L2, R2, ...]`

### Per-Channel State

Each instrument can play on 8 channels simultaneously. Track state per channel:

```cpp
struct ChannelState {
    bool active;
    unsigned char note;
    // Add per-channel synthesis state
};

ChannelState channels_[8];
```

## Testing Your Synth

### 1. Compile LGPT

```bash
cd projects/
make PLATFORM=MIYOO  # Or your platform
```

### 2. Test Wavetable Instrument

1. Launch LGPT
2. Navigate to Instrument screen (RT+Right)
3. Select instrument type → SYNTH
4. Press note to trigger
5. Adjust waveform parameter

### 3. Debug Audio Output

```cpp
// Add debug logging in Render()
printf("Channel %d rendering: phase=%d note=%d\n",
       channel, fp2i(ch.phase), ch.note);
```

## Performance Considerations

### CPU Budget

**RG Nano specs:**
- ARM Cortex-A7 @ 1.2 GHz
- 8 simultaneous channels

**Budget per channel per callback:**
- ~200 µs per channel (44100 Hz / 8 channels)
- Simple wavetable: ~10-20 µs
- FM synthesis (2-op): ~50-100 µs
- Braids algorithms: ~100-200 µs

### Optimization Tips

1. **Use lookup tables** for expensive math (sin, exp)
2. **Integer math** where possible
3. **Inline critical functions**
4. **Profile** with real hardware

```cpp
// Good: Lookup table
short sine_table[256];

// Bad: Runtime calculation
sample = (short)(sin(phase) * 32767);
```

## Reusing Existing LGPT Features

### Filters

`SampleInstrument` has filters you can reuse:

```cpp
// In your Render() method, after generating samples:
applyFilter(buffer, size, cutoff, resonance);
```

### Envelopes

Study `SampleInstrument`'s envelope implementation:
- `sources/Application/Instruments/SRPUpdaters.h` - Envelope updaters
- Apply volume modulation based on attack/decay/sustain/release

### Effects

LGPT's command system (`ProcessCommand`) gives you:
- Volume
- Pan
- Filters
- Custom FX commands

## Example: Complete Sine Wave Instrument

Minimal working synth (200 lines):

```cpp
// SimpleSynth.h
class SimpleSynth : public I_Instrument {
    struct Channel {
        bool active;
        fixed phase, phaseStep;
        int volume;
    } ch_[8];

    virtual bool Render(int channel, fixed *buffer, int size, bool updateTick) {
        if (!ch_[channel].active) return false;

        for (int i = 0; i < size; i++) {
            // Sine wave generation
            float p = fp2f(ch_[channel].phase);
            short sample = (short)(sin(p * 2.0 * M_PI) * 32767.0 * ch_[channel].volume / 255);

            buffer[i*2] = buffer[i*2+1] = i2fp(sample);

            // Advance phase
            ch_[channel].phase += ch_[channel].phaseStep;
            if (ch_[channel].phase >= i2fp(1))
                ch_[channel].phase -= i2fp(1);
        }
        return true;
    }
};
```

## Resources

### LGPT Codebase Structure
- `sources/Application/Instruments/` - Instrument implementations
- `sources/Application/Mixer/` - Audio mixing
- `sources/Services/Audio/` - Low-level audio I/O
- `sources/Foundation/` - Utility classes

### External Libraries to Consider
- **STK (Synthesis Toolkit)** - Various synthesis algorithms
- **Mutable Instruments** - Eurorack DSP code (Braids, Rings, Elements)
- **MusicDSP** - Algorithm archive

### Learning Resources
- **The Audio Programming Book** - DSP fundamentals
- **Designing Sound** by Andy Farnell - Synthesis techniques
- **JUCE tutorials** - Modern C++ audio programming

## Next Steps

1. **Start simple** - Get sine wave working first
2. **Test on hardware** - RG Nano performance testing
3. **Iterate** - Add features incrementally
4. **Share** - First LGPT synth implementation would be novel!

## FAQ

**Q: Can I use floating-point math?**
A: Yes, but fixed-point is faster on ARM. Profile first.

**Q: How do I access LGPT's sample data from a synth?**
A: You could hybrid - use samples as wavetables. Access via `SamplePool`.

**Q: Will this work on other platforms (PSP, Dingoo, etc)?**
A: Yes! LGPT is cross-platform. Test on multiple devices.

**Q: Can I combine synthesis and sampling?**
A: Absolutely - create a hybrid instrument that does both.

---

**This is a genuinely novel contribution to LGPT.** No one has added synthesis engines before. You'd be breaking new ground!
