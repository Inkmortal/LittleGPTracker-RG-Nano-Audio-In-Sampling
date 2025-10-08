# LGPT Audio Input Modification for RG Nano

This fork adds **live audio recording** capability to LittleGPTracker (LGPT) for use with USB-C audio adapters on the RG Nano handheld.

## What's New

### Features Added
- **Live audio capture** via USB-C audio adapters
- **Recording dialog** accessible from Instrument screen
- **Auto-import** recorded samples into current instrument
- **Up to 60 seconds** recording per take (configurable)
- **Stereo 44.1kHz 16-bit** audio quality
- **Non-destructive trimming** using LGPT's built-in start/end points

## How to Use

### Prerequisites
1. **USB-C to 3.5mm audio adapter** (UAC compliant - dual jack recommended)
2. **FunKey-OS with USB audio enabled** in kernel (`CONFIG_SND_USB_AUDIO=y`)
3. **Audio source** (synth, mic, line-out from another device)

### Recording Workflow

1. **Navigate to Instrument screen** (RT+Right from Song/Chain/Phrase screens)
2. **Select the "sample" parameter** (first line)
3. **Press LT+A** (Left shoulder + A) to open recording dialog
4. **Adjust duration** with UP/DOWN (1-60 seconds)
5. **Press START** to begin recording
6. **Press START again** to stop early (or wait for auto-stop)
7. **Press A** to save and auto-import to current instrument
8. **Sample is now loaded** and ready to sequence!
9. **Optional: Trim the sample** using start/end parameters in Instrument screen

### Button Mappings
- **LT+A** on sample parameter → Open recording dialog
- **START** in recording dialog → Start/stop recording
- **UP/DOWN** → Adjust max recording duration (1-60 seconds)
- **A** → Save and import recording
- **B** → Cancel recording

### Trimming Recorded Samples
After recording, you can non-destructively trim samples using LGPT's built-in parameters:
- **start** - Set playback start point (in hex)
- **end** - Set playback end point (in hex)
- **loop start** - Set loop start point
- **loop end** - Set loop end point

This means you can:
- Record a long take (up to 60s)
- Find the perfect section using start/end points
- Set loop points for seamless loops
- All without re-recording!

## Technical Details

### New Files Added
```
sources/Adapters/SDL/Audio/SDLAudioCapture.h
sources/Adapters/SDL/Audio/SDLAudioCapture.cpp
sources/Application/Views/ModalDialogs/RecordSampleDialog.h
sources/Application/Views/ModalDialogs/RecordSampleDialog.cpp
```

### Modified Files
```
sources/Application/Views/InstrumentView.cpp  # Added button mapping
projects/Makefile                             # Added new object files
```

### Architecture
- **SDLAudioCapture**: Low-level SDL audio capture using callback API
- **RecordSampleDialog**: UI for recording workflow
- **WavFileWriter**: Existing class used to save recordings as WAV files
- **SamplePool**: Existing class used to import saved files

### Audio Format
- **Sample Rate**: 44100 Hz
- **Bit Depth**: 16-bit signed
- **Channels**: Stereo (2)
- **Format**: PCM WAV

### Saved Files
Recordings are saved in your project's `samples/` directory with auto-generated filenames:
```
samples/recorded_001.wav
samples/recorded_002.wav
samples/recorded_003.wav
...
```

## Building

### For RG Nano (Miyoo-based build)
```bash
cd projects
make PLATFORM=MIYOO
```

### For Desktop Testing
```bash
cd projects

# Linux x64
make PLATFORM=X64

# Linux x86 (32-bit)
make PLATFORM=X86

# Debian
make PLATFORM=DEB
```

## Kernel Setup (FunKey-OS)

To enable USB audio input on RG Nano with FunKey-OS:

1. Edit `FunKey/board/funkey/linux.config`:
```
CONFIG_SND_USB=y
CONFIG_SND_USB_AUDIO=y
```

2. Rebuild kernel:
```bash
make FunKey/linux-rebuild
make image update
```

3. Flash to SD card or use firmware update

## Troubleshooting

### "Failed to open audio capture device"
- Ensure USB audio adapter is plugged in
- Check kernel has `CONFIG_SND_USB_AUDIO=y`
- Verify with `arecord -l` on device

### No audio captured / silent recording
- Check adapter is UAC compliant
- Try different USB-C adapter
- Verify input levels (some adapters have very low gain)

### Recordings sound distorted
- Reduce input source volume
- Use adapter with separate mic/line jacks
- Check for ground loop issues

### Want longer recording times?
Edit `sources/Adapters/SDL/Audio/SDLAudioCapture.h`:
```cpp
#define MAX_RECORDING_SECONDS 60  // Change to 120, 180, etc.
```
**Note**: Each 60 seconds uses ~10MB RAM. RG Nano has limited memory!

## Hardware Recommendations

### USB-C Audio Adapters That Work
- **UGREEN USB-C to dual 3.5mm** (mic + headphone)
- **Generic USB sound cards** with C-Media CM108/CM119 chipsets
- Any **UAC 1.0 or 2.0 compliant** adapters

### Avoid
- Apple USB-C to 3.5mm dongle (analog passthrough, not digital UAC)
- Cheap passive adapters
- Proprietary phone-specific adapters

## Credits

Based on **LittleGPTracker** by djdiskmachine and original author m-.-n
Audio input modification by Claude Code
For use with **FunKey-OS** on RG Nano hardware

## License

GPLv3 (same as original LGPT)
