# LittleGPTracker - RG Nano with Audio Input Sampling

This is a modified version of LittleGPTracker (LGPT) that adds **live audio input sampling** capability for the RG Nano handheld console.

## What's New

### Audio Input Sampling
- **Live recording** via USB-C audio adapters
- **44.1kHz 16-bit stereo** capture
- **Up to 60 seconds** recording time
- **Real-time monitoring** during recording
- **Instant import** to instrument slots

### Modified Files

**New Files:**
- `sources/Adapters/SDL/Audio/SDLAudioCapture.h` - Audio capture driver interface
- `sources/Adapters/SDL/Audio/SDLAudioCapture.cpp` - SDL audio capture implementation
- `sources/Application/Views/ModalDialogs/RecordSampleDialog.h` - Recording UI header
- `sources/Application/Views/ModalDialogs/RecordSampleDialog.cpp` - Recording UI implementation

**Modified Files:**
- `sources/Application/Views/ModalDialogs/ImportSampleDialog.cpp` - Added "Record" button to sample browser
- `sources/Application/Views/ModalDialogs/RecordSampleDialog.cpp` - Updated to save to `recorded/` subfolder with short names
- `projects/Makefile` - Added new source files to build

## Hardware Requirements

### RG Nano Console
- **CPU**: Allwinner V3s ARM Cortex-A7 @ 1.2GHz
- **Audio**: Modified FunKey-OS with USB audio input support
- **OS**: Custom FunKey-OS build with kernel audio drivers

### USB Audio Adapter
- **Type**: USB-C to 3.5mm TRS adapter
- **Chipset**: UAC compliant (C-Media CM108/CM119 or similar)
- **Channels**: Stereo input
- **NOT compatible**: Apple dongles (analog passthrough only)

**Recommended adapters:**
- UGREEN USB-C to dual 3.5mm (separate mic/headphone)
- Generic UAC 1.0/2.0 compliant adapters with digital chips

## How to Use

### 1. Setup Hardware
1. Flash modified FunKey-OS with USB audio support (see FunKey-OS-Test-Audio-In-For-LGPT project)
2. Connect USB-C audio adapter to RG Nano
3. Connect audio source (synth, microphone, line-out) to adapter

### 2. Launch LGPT
1. Boot RG Nano
2. Navigate to Applications
3. Launch LGPT

### 3. Record Audio
1. Navigate to **Instrument screen** (RT + Right)
2. Select the **"sample"** parameter (first line)
3. Press **A** to enter sample selection mode
4. Press **A** again to open the sample browser dialog
5. Use **LEFT/RIGHT** to navigate to **"Record"** button
6. Press **A** to open recording dialog
7. Adjust duration with **UP/DOWN** (1-30 seconds, default 5)
8. Press **START** to begin recording
9. Press **START** again to stop (or wait for auto-stop)
10. Press **A** to save and auto-import sample
11. Sample is now loaded into current instrument!

**Sample Browser Buttons:**
- **Listen** - Preview WAV file
- **Import** - Import existing WAV file
- **Record** - Record new audio sample
- **Exit** - Close browser

### 4. Edit Sample
Once recorded:
- Use **start/end** parameters to trim
- Adjust **loop points** for looping
- Apply **filters**, **crush**, **downsample** effects
- Use in patterns/chains as normal

## Button Mappings

| Action | Buttons | Context |
|--------|---------|---------|
| Open sample browser | **A** (twice) | Instrument screen, sample parameter selected |
| Navigate browser buttons | **LEFT/RIGHT** | Sample browser dialog |
| Select Record button | **A** | Sample browser (Record button selected) |
| Start/stop recording | **START** | Recording dialog |
| Adjust duration | **UP/DOWN** | Recording dialog |
| Save recording | **A** | Recording dialog (after recording) |
| Cancel | **B** | Recording dialog or sample browser |

## Technical Details

### Audio Capture Flow
```
USB Audio Adapter
    ↓
FunKey-OS Kernel (USB Audio Class driver)
    ↓
ALSA (hw:1,0)
    ↓
SDL Audio Capture (callback API)
    ↓
SDLAudioCapture class
    ↓
RecordSampleDialog (UI)
    ↓
WavFileWriter (save to disk)
    ↓
SamplePool (import to instrument)
```

### File Locations
- **Recorded samples**: `project:samples/recorded/rec_001.wav`, `rec_002.wav`, etc.
- **Subfolder**: Auto-created on first recording
- **Naming**: Short sequential names (`rec_XXX.wav`)
- **Project samples**: Loaded automatically when project opens
- **Instrument assignments**: Saved with project

### Memory Considerations
- **60 seconds @ 44.1kHz stereo 16-bit** = ~10MB RAM
- RG Nano has limited memory
- Delete unused samples regularly
- Use start/end trimming instead of re-recording

### Sample Format
- **Rate**: 44100 Hz
- **Channels**: 2 (stereo)
- **Bit depth**: 16-bit signed PCM
- **Endianness**: Little-endian
- **Format**: WAV (RIFF)

## Building from Source

### Prerequisites
- FunKey SDK (ARM cross-compiler)
- SDL 1.2 development libraries
- ALSA development libraries

### Build Commands
```bash
cd projects/
make PLATFORM=MIYOO clean
make PLATFORM=MIYOO
```

### Build Output
- **Binary**: `lgpt` executable
- **OPK Package**: `lgpt.opk` (for RG Nano installation)

### Installing on RG Nano
1. Copy `lgpt.opk` to `/usr/local/share/OPKs/Applications/`
2. Reboot or refresh menu
3. Launch from Applications menu

## Architecture Overview

### SDLAudioCapture Class
```cpp
class SDLAudioCapture {
public:
    bool Initialize(int sampleRate, int channels);
    bool StartRecording();
    bool StopRecording();
    bool IsRecording();
    int GetRecordedSampleCount();
    void CopyRecordedData(short* destination, int sampleCount);
    void ClearBuffer();
private:
    static void audioCallback(void* userdata, Uint8* stream, int len);
    std::vector<short> recordBuffer_;
    bool isRecording_;
};
```

**Key Features:**
- Ring buffer for continuous capture
- SDL audio callback for low-latency
- Thread-safe recording state
- Automatic buffer management

### RecordSampleDialog Class
```cpp
class RecordSampleDialog : public ModalDialog {
public:
    virtual void OnPlayerUpdate();  // Update during recording
    virtual void ProcessButtonMask(unsigned short mask);
private:
    SDLAudioCapture audioCapture_;
    int recordDuration_;  // In seconds
    int recordedSamples_;
    enum State { READY, RECORDING, RECORDED };
};
```

**Features:**
- Real-time recording progress display
- Duration selection (1-60 seconds)
- Auto-stop on timeout
- Instant save and import

### Integration Points

**ImportSampleDialog.cpp** (4-button layout):
```cpp
static const char *buttonText[4]= {
    "Listen",
    "Import",
    "Record",  // NEW: Added record button
    "Exit"
};

// Button handler
switch(selected_) {
    case 0: // Listen (preview)
        preview(*element);
        break;
    case 1: // Import
        import(*element);
        break;
    case 2: // Record (NEW)
        endPreview();
        RecordSampleDialog *rsd = new RecordSampleDialog(*this);
        DoModal(rsd);
        break;
    case 3: // Exit
        EndModal(0);
        break;
}
```

**RecordSampleDialog.cpp** (file naming):
```cpp
std::string RecordSampleDialog::generateFileName() {
    static int counter = 1;
    std::ostringstream oss;
    oss << "rec_" << std::setfill('0') << std::setw(3) << counter++ << ".wav";
    return oss.str();  // Returns "rec_001.wav", "rec_002.wav", etc.
}

// Auto-create subfolder and save
std::string recordedFolder = std::string(samplePath) + "/recorded";
FileSystem::GetInstance()->MakeDir(recordedFolder.c_str());
std::string fullPath = recordedFolder + "/" + fileName_;
```

## Troubleshooting

### No Audio Captured / Silent Recordings

**Check ALSA levels:**
```bash
# SSH into RG Nano
ssh root@10.42.0.1

# Check capture devices
arecord -l

# Adjust input levels
alsamixer -c 1  # USB device is usually card 1
# Navigate to "Capture" or "Mic" controls
# Press SPACE to enable capture
# Use UP/DOWN to adjust level
```

**Test capture directly:**
```bash
arecord -D hw:1,0 -f S16_LE -r 44100 -c 2 -d 5 test.wav
aplay test.wav
```

### USB Device Not Detected

**Check kernel modules:**
```bash
lsmod | grep snd_usb_audio
```

**Check USB devices:**
```bash
lsusb
dmesg | grep -i usb
```

**Verify kernel config:**
- `CONFIG_SND_USB=y`
- `CONFIG_SND_USB_AUDIO=y`
- `CONFIG_USB_MUSB_DUAL_ROLE=y`

### Recording Dialog Won't Open

**Requirements:**
- Must be on Instrument screen (RT+Right)
- Sample parameter must be selected (top line)
- Press **A** twice to open sample browser
- Use **LEFT/RIGHT** to navigate to **"Record"** button
- Press **A** to open recording dialog
- Instrument type must be SAMPLE

**Debug:**
Check build includes new files:
```bash
nm lgpt | grep RecordSampleDialog
nm lgpt | grep ImportSampleDialog
```

**Common issues:**
- Make sure you're selecting the "Record" button (3rd button from left)
- The sample browser shows: [Listen] [Import] [Record] [Exit]

### Distorted / Clipped Audio

**Reduce input levels:**
```bash
alsamixer -c 1
# Lower "Mic" or "Capture" levels
```

**Reduce source output:**
- Turn down synth/device output volume
- Use line-level instead of headphone-level signals

### Buffer Overruns / Dropouts

**Symptoms:**
- Clicking/popping in recordings
- Glitches or gaps

**Solutions:**
- Close other applications on RG Nano
- Reduce recording duration
- Use lower-quality audio adapter
- Check USB cable quality

## Performance Notes

### CPU Usage
- **Recording**: ~5-10% CPU (SDL callback)
- **Playback**: Same as regular sample playback
- **Both**: ~15-20% CPU total

### Memory Usage
| Duration | RAM Usage |
|----------|-----------|
| 10 sec | ~1.7 MB |
| 30 sec | ~5.1 MB |
| 60 sec | ~10.2 MB |

### Latency
- **Input latency**: ~50-100ms (SDL + ALSA + USB)
- **Not suitable for**: Real-time monitoring during performance
- **Good for**: Sampling before jamming

## Future Enhancements

Potential improvements:
- [ ] Real-time waveform visualization during recording
- [ ] Auto-normalize recordings
- [ ] Compression/gain adjustment
- [ ] Mono recording option (save memory)
- [ ] Multiple recording formats
- [ ] Direct loop point detection
- [ ] Sample rate conversion options

## Credits

**Original LittleGPTracker:**
- m-.-n (Marc Nostromo) - Original author
- djdiskmachine - Current maintainer
- LGPT community

**Audio Input Modifications:**
- Created for RG Nano platform
- Based on SDL audio capture API
- Integrates with existing LGPT architecture

**Hardware:**
- RG Nano / FunKey-S console
- FunKey-OS by DrUm78

## License

Same as LittleGPTracker: **GPL-3.0**

This modification follows LGPT's open-source license and contributes back to the community.

## Related Documentation

- **ADDING_SYNTH_INSTRUMENTS.md** - Guide to adding synthesis engines to LGPT
- **FunKey-OS modifications** - See ../FunKey-OS-Test-Audio-In-For-LGPT/

## Support

**For LGPT audio input issues:**
- Test USB audio outside LGPT first (`arecord`)
- Check FunKey-OS kernel has USB audio enabled
- Verify adapter is UAC compliant

**For general LGPT issues:**
- https://github.com/Mdashdotdashn/LittleGPTracker
- LGPT forums and community

**For FunKey-OS issues:**
- https://github.com/DrUm78/FunKey-OS

---

**Happy sampling!** 🎵
