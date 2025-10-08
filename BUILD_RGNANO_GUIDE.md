# Building LGPT for RG Nano - Complete Guide

## Status: ✅ BUILD SUCCESSFUL

**Binary created:** `lgpt-rgnano.elf` (517KB)
**Architecture:** ARM 32-bit, ARMv7-A Cortex-A7
**Platform:** FunKey-OS (SDL1 + ALSA)

---

## Quick Build

```bash
cd projects
make PLATFORM=RGNANO
```

Output: `lgpt-rgnano.elf`

---

## What's Included

### UX Improvements (All Implemented)
1. ✅ Cursor pulse animation (View.h/cpp)
2. ✅ Split-row mode for 8 channels (SongView.h/cpp)
3. ✅ Channel meters in Chain view (ChainView.h/cpp)
4. ✅ Mode indicator (1/2, 2/2)
5. ✅ Audio input sampling via ALSA (ALSAAudioCapture.cpp)

### Platform Configuration
- **Base:** DINGUX platform (SDL1)
- **Audio Out:** SDL1 (`SDLAudio.cpp`)
- **Audio In:** ALSA (`ALSAAudioCapture.cpp`) - for sampling
- **MIDI:** Dummy (no hardware MIDI)
- **System:** RGNANOSystem.cpp

---

## Build System Configuration

### Files Modified

#### 1. `projects/Makefile`

**Line 10-14:** Platform makefile include (uses ROOT_DIR pattern)
```makefile
MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT_DIR := $(dir $(MAKEFILE_PATH))

export PLATFORM
-include $(ROOT_DIR)Makefile.$(PLATFORM)
```

**Line 168:** Directory configuration (after MIYOODIRS)
```makefile
RGNANODIRS := $(DINGUXDIRS) $(DUMMYMIDIDIRS) $(ALSADIRS)
```

**Line 435-440:** File list (after MIYOOFILES)
```makefile
RGNANOFILES := \
	$(DINGUXFILES) \
	DummyMidi.o \
	RGNANOSystem.o \
	SDLAudio.o \
	ALSAAudioCapture.o
```

#### 2. `projects/Makefile.RGNANO`

Platform-specific build configuration using ROOT_DIR:

```makefile
-include $(ROOT_DIR)rules_base

DEFINES := \
	-DPLATFORM_$(PLATFORM) \
	-DBUFFERED \
	-DCPP_MEMORY \
	-DHAVE_STDINT_H \
	-D_NDEBUG \
	-D_NO_JACK_

SDK := $(ROOT_DIR)../sdk/FunKey-sdk-DrUm78
SYSROOT := $(SDK)/arm-funkey-linux-musleabihf/sysroot
CROSS_COMPILE := $(SDK)/bin/arm-funkey-linux-musleabihf-

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
SDL_CFLAGS  := -I$(SYSROOT)/usr/include/SDL -D_REENTRANT
SDL_LIBS    := -lSDL

INCLUDES = -Iinclude $(SDL_CFLAGS) -I$(ROOT_DIR)../sources
OPT_FLAGS  = -O3 -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard

TOOLPATH=$(SDK)/bin
PREFIX		:=	arm-funkey-linux-musleabihf-

CFLAGS	:=	$(DEFINES) $(INCLUDES) $(SDL_CFLAGS) $(OPT_FLAGS) -Wall
CXXFLAGS:=	$(CFLAGS) -std=gnu++03
LDFLAGS := -L$(SYSROOT)/usr/lib
LIBS	:=  -lSDL -lasound -lpthread
OUTPUT = ../lgpt-rgnano
EXTENSION:= elf

%.elf: $(OFILES)
	$(CXX) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)
	@$(STRIP) $@
```

#### 3. Platform-Specific Source Files

**Created:**
- `sources/Adapters/DINGOO/System/RGNANOSystem.cpp` - Platform initialization
- `sources/Adapters/ALSA/Audio/ALSAAudioCapture.cpp` - ALSA audio input for sampling
- `sources/Adapters/ALSA/Audio/ALSAAudioCapture.h`

---

## Build System Details

### Critical Build Pattern

**IMPORTANT:** Variable definitions must follow dependency order:

```makefile
# Base definitions first (line 145):
DINGUXFILES := DINGOOEventQueue.o GPSDLMain.o ...

# Platform files AFTER base (line 435+):
RGNANOFILES := $(DINGUXFILES) DummyMidi.o RGNANOSystem.o ...
```

**Why:** Make uses immediate expansion (`:=`). If `RGNANOFILES` is defined before `DINGUXFILES`, the `$(DINGUXFILES)` reference expands to empty.

### Recursive Make Pattern

The build uses recursive make:
1. **Initial make:** Creates build directory, sets up variables
2. **Recursive make:** Changes to `buildRGNANO/`, re-reads Makefile with `-f $(PWD)/Makefile`

**ROOT_DIR Solution:** Ensures platform makefiles are found in both contexts:
- Initial: `ROOT_DIR = /path/to/projects/`
- Recursive: `ROOT_DIR = /path/to/projects/` (same, using `$(abspath $(lastword $(MAKEFILE_LIST)))`)

---

## Object Files Compiled

**Total: 147 files**
- 132 common application files
- 11 DINGUX platform files (SDL1 base)
- 4 RG Nano-specific files

### Platform Files (DINGUXFILES + RG Nano)
```
DINGOOEventQueue.o      - Event handling
GPSDLMain.o             - Main entry point
GUIFactory.o            - GUI creation
Process.o               - Process management
SDLAudioDriver.o        - SDL audio driver
SDLEventManager.o       - Event management
SDLGUIWindowImp.o       - Window implementation
SDLProcess.o            - SDL process handling
SDLTimer.o              - Timing
UnixFileSystem.o        - File I/O
UnixProcess.o           - Unix process support
DummyMidi.o             - Stub MIDI (no hardware)
RGNANOSystem.o          - RG Nano platform init
SDLAudio.o              - Audio output
ALSAAudioCapture.o      - Audio input (sampling)
```

---

## SDK Setup

SDK location: `../sdk/FunKey-sdk-DrUm78/`

Includes:
- ✅ Compiler: `bin/arm-funkey-linux-musleabihf-g++`
- ✅ SDL1 headers: `arm-funkey-linux-musleabihf/sysroot/usr/include/SDL/`
- ✅ SDL1 libs: `arm-funkey-linux-musleabihf/sysroot/usr/lib/libSDL.so`
- ✅ ALSA libs: `arm-funkey-linux-musleabihf/sysroot/usr/lib/libasound.so`

---

## Controls (Implemented in Code)

- **L+SELECT**: Toggle split-row ↔ paged mode
- **L+Left/Right**: Manual channel bank switching (paged mode)
- Auto-paging follows cursor automatically

---

## Build Output

```
Compiling 147 object files...
Linking lgpt-rgnano.elf...
Stripping debug symbols...

Output: lgpt-rgnano.elf (517KB)
```

**Compiler flags:**
- `-O3` - Full optimization
- `-march=armv7-a -mtune=cortex-a7` - ARMv7-A Cortex-A7 CPU
- `-mfpu=neon-vfpv4` - NEON SIMD extensions
- `-mfloat-abi=hard` - Hardware floating point

**Linked libraries:**
- `-lSDL` - SDL 1.2 for video/audio output
- `-lasound` - ALSA for audio input (sampling)
- `-lpthread` - Threading support

---

## Troubleshooting

### Issue: "No rule to make target 'XXXSystem.o'"

**Cause:** Platform file list defined before base file list in Makefile

**Fix:** Ensure `RGNANOFILES` is defined AFTER `DINGUXFILES` (around line 435+)

### Issue: "cannot find -lSDL"

**Cause:** SDK not extracted or wrong path

**Fix:** Verify SDK at `../sdk/FunKey-sdk-DrUm78/`

### Issue: Variables empty in recursive make

**Cause:** Platform makefile not being included correctly

**Fix:** Verify `ROOT_DIR` is set and include uses `$(ROOT_DIR)Makefile.$(PLATFORM)`

---

## Next Steps

1. ✅ Binary built successfully
2. 🔄 Test on RG Nano hardware
3. 🔄 Verify UX improvements (split-row, meters, animation)
4. 🔄 Test audio input sampling functionality
5. 🔄 Package as OPK for easy installation

---

## Technical Notes

### Why SDL1 instead of SDL2?

FunKey-OS (RG Nano) uses SDL 1.2. The MIYOO platform already had working SDL1 config, so RGNANO follows the same pattern.

### Why ALSA for audio input?

SDL1 doesn't have audio capture API. ALSA provides direct access to the RG Nano's audio input for sampling.

### Build System Pattern

Based on examination of MIYOO, BITTBOY, and GARLIC platforms. All DINGUX-based devices follow this pattern:
- Base: `DINGUXFILES` + `DINGUXDIRS`
- Platform: Add system file + audio implementation
- Libraries: SDL1 + optional (ALSA, SDL_mixer, etc.)

---

**Build verified:** October 6, 2025
**Status:** Ready for hardware testing ✅
