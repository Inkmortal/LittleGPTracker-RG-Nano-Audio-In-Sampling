# RG Nano Channel Display Options

## The Challenge
- **RG Nano Screen**: 30 columns × 30 rows
- **Song View Needs**: 8 channels @ 3 chars each = 24 chars minimum
- **Available**: 30 columns - 6 (header/margins) = ~24 columns usable

## Option 1: Two-Row Display (YOUR PREFERENCE)

### Visual Layout
```
SONG                        (30 cols)
00│01 02 03 04             Row 1: Channels 1-4
  │05 06 07 08             Row 2: Channels 5-8
01│0A 0B 0C 0D
  │0E 0F 10 11
02│-- -- -- --
  │-- -- -- --
```

### Pros ✅
1. **See ALL channels at once** - No hidden information
2. **Faster workflow** - No scrolling/paging needed
3. **Better overview** - See full song arrangement
4. **Muscle memory preserved** - All channels always visible
5. **Vertical space available** - RG Nano is square (30×30)
6. **Easier selection** - Just up/down between channel rows

### Cons ⚠️
1. **Unusual layout** - Not standard tracker format
2. **Row confusion** - Is this row 00 or 01?
3. **Less pattern rows visible** - Using 2 lines per song row = half the visible rows
4. **Selection complexity** - Selecting across split rows tricky
5. **Cursor positioning** - Need to track which "sub-row"

### Current Visible Rows
```
Standard 30×30:
- Header: 1 row
- Song rows: 29 rows (normally ~22 pattern rows)

Two-row mode:
- Header: 1 row
- Song rows: 28 rows ÷ 2 = 14 pattern rows visible

Loss: 8 pattern rows (36% reduction)
```

## Option 2: Channel Paging (4+4)

### Visual Layout
```
SONG                   <1/2>  (30 cols)
00│01 02 03 04            Channels 1-4
01│0A 0B 0C 0D
02│-- -- -- --
[... 22+ visible rows ...]

Press L+Right →

SONG                   <2/2>
00│05 06 07 08            Channels 5-8
01│0E 0F 10 11
02│-- -- -- --
[... 22+ visible rows ...]
```

### Pros ✅
1. **Standard layout** - Looks like normal tracker
2. **More pattern rows** - Full 22+ rows visible
3. **Clear focus** - Only 4 channels to focus on
4. **Simpler rendering** - No split-row logic
5. **Familiar UX** - Similar to DAW channel banks

### Cons ⚠️
1. **Hidden channels** - Can't see all 8 at once
2. **Navigation overhead** - Press L+Right/Left to switch
3. **Lost context** - What's happening in other 4 channels?
4. **Slower workflow** - Constant page switching
5. **Mute confusion** - Which page has muted channels?

## Option 3: Hybrid Mode (BEST OF BOTH WORLDS)

### Implementation
```cpp
enum ChannelDisplayMode {
    SPLIT_ROW,    // All 8 channels, 2 rows each
    PAGED_4       // 4 channels at a time, full rows
};

// User toggles with SELECT+L or config setting
```

### Default Recommendations
- **Song editing**: Split-row mode (see everything)
- **Live performance**: Paged mode (clearer focus)
- **Complex patterns**: Split-row mode (context matters)
- **Simple beats**: Either works

## Detailed Comparison

### Vertical Space Analysis

#### Split-Row Mode
```
Row layout:
00│01 02 03 04    ← Song row 00, channels 1-4
  │05 06 07 08    ← Song row 00, channels 5-8
01│0A 0B 0C 0D    ← Song row 01, channels 1-4
  │0E 0F 10 11    ← Song row 01, channels 5-8

Visible song rows: 14
```

#### Paged Mode
```
Row layout:
00│01 02 03 04    ← Song row 00, channels 1-4
01│0A 0B 0C 0D    ← Song row 01, channels 1-4
02│-- -- -- --    ← Song row 02, channels 1-4

Visible song rows: 28
```

**Winner**: Paged mode (2× more pattern rows visible)

### Information Density

#### Split-Row Mode
- **Channels visible**: 8/8 (100%)
- **Pattern rows**: 14
- **Total cells visible**: 8 × 14 = 112

#### Paged Mode
- **Channels visible**: 4/8 (50%)
- **Pattern rows**: 28
- **Total cells visible**: 4 × 28 = 112

**Winner**: TIE (same number of cells)

### Workflow Comparison

#### Adding a bass line across 8 bars
**Split-row**:
1. Navigate to channel 2
2. Enter patterns 00-07
3. Done - all visible

**Paged**:
1. Navigate to channel 2 (page 1)
2. Enter patterns 00-07
3. Done - but might need to check page 2 for other channels

**Winner**: Split-row (no page switching)

#### Adjusting drums on channel 8
**Split-row**:
1. Navigate down to channel 8 (bottom row)
2. Edit patterns
3. See channels 1-7 above for context

**Paged**:
1. Press L+Right to page 2
2. Navigate to channel 8
3. Edit patterns
4. Can't see channels 1-4

**Winner**: Split-row (context preserved)

#### Live muting during performance
**Split-row**:
1. RT+L on any channel
2. All muted channels visible with indicators

**Paged**:
1. RT+L on any channel
2. Might need to switch pages to see which channels muted

**Winner**: Split-row (better visual feedback)

## Technical Implementation

### Split-Row Mode
```cpp
// SongView.cpp - Modified drawing
void SongView::DrawSongRow(int row, GUIPoint anchor) {
    // Draw channels 1-4 on first line
    for (int ch = 0; ch < 4; ch++) {
        DrawPattern(anchor._x + ch*3, anchor._y, row, ch);
    }

    // Draw channels 5-8 on second line (indented)
    for (int ch = 4; ch < 8; ch++) {
        DrawPattern(anchor._x + (ch-4)*3, anchor._y + 1, row, ch);
    }
}

// Cursor navigation
void SongView::OnPlayerUpdate() {
    if (cursor_y_ > 3) {
        // Bottom half - channels 5-8
        visual_y_ = (cursor_y_ * 2) + 1;
    } else {
        // Top half - channels 1-4
        visual_y_ = cursor_y_ * 2;
    }
}
```

### Paged Mode
```cpp
// SongView.cpp - Channel banking with auto-paging
class SongView {
    int channelBank_;  // 0 = channels 0-3, 1 = channels 4-7
    bool manualBankOverride_;  // User manually switched banks

    void OnCursorMove() {
        if (!manualBankOverride_) {
            // Auto-page based on cursor position
            channelBank_ = (viewData_->songX_ >= 4) ? 1 : 0;
        }
    }

    void OnButtonPressed(int button) {
        // Manual bank switching: L + Left/Right
        if (button == EPBM_L + EPBM_RIGHT) {
            channelBank_ = (channelBank_ + 1) % 2;
            manualBankOverride_ = true;
        }
        if (button == EPBM_L + EPBM_LEFT) {
            channelBank_ = (channelBank_ - 1 + 2) % 2;
            manualBankOverride_ = true;
        }

        // Any cursor movement clears manual override
        if (button & (EPBM_UP | EPBM_DOWN | EPBM_LEFT | EPBM_RIGHT)) {
            manualBankOverride_ = false;
        }
    }

    void DrawSongRow(int row, GUIPoint anchor) {
        int startCh = channelBank_ * 4;
        for (int i = 0; i < 4; i++) {
            DrawPattern(anchor._x + i*3, anchor._y, row, startCh + i);
        }
    }
};
```

### Hybrid Toggle
```cpp
// Config.xml
<CHANNELMODE value="SPLIT"/>  <!-- or "PAGED" -->

// Runtime toggle: L + SELECT
if (L + SELECT pressed) {
    channelMode = (channelMode == SPLIT) ? PAGED : SPLIT;
    SavePreference();
    ShowToast(channelMode == SPLIT ? "SPLIT MODE" : "PAGED 1/2");
}

// In paged mode: Auto-paging + manual override
if (channelMode == PAGED) {
    // Auto-page follows cursor
    if (!manualOverride) {
        channelBank = (cursor_x >= 4) ? 1 : 0;
    }

    // Manual override: L + Left/Right
    if (L + RIGHT pressed) {
        channelBank = 1;
        manualOverride = true;
    }
    if (L + LEFT pressed) {
        channelBank = 0;
        manualOverride = true;
    }

    // Clear override on cursor move
    if (cursor moved) {
        manualOverride = false;
    }
}
```

## User Preference Survey (Hypothetical)

Based on typical tracker users:

### Power Users (40%)
- **Prefer**: Split-row
- **Reason**: Need to see everything at once
- **Quote**: "I need context - what's kick doing while bass plays?"

### Beginners (30%)
- **Prefer**: Paged
- **Reason**: Less overwhelming
- **Quote**: "I only work on 2-3 channels at a time anyway"

### Live Performers (20%)
- **Prefer**: Split-row
- **Reason**: Quick mute/unmute visibility
- **Quote**: "I need to see which channels are playing"

### Mobile Users (10%)
- **Prefer**: Paged
- **Reason**: Easier to read on tiny screen
- **Quote**: "Small screen + 8 channels = squinting"

## Recommendation: HYBRID WITH SMART DEFAULTS

### Default Mode Selection
```cpp
// Auto-select based on context
if (isLiveMode || hasMutedChannels) {
    defaultMode = SPLIT_ROW;  // Need visibility
} else if (isEditing && focusedChannels < 5) {
    defaultMode = PAGED;       // Cleaner focus
} else {
    defaultMode = userPreference;  // Respect setting
}
```

### Visual Indicators
```
Split-row mode:
SONG [━━━━━━━━] ALL     ← Shows all 8 channels

Paged mode:
SONG [━━━━░░░░] 1/2     ← Shows 4/8, page 1 of 2
```

### Quick Toggle
- **L + SELECT**: Toggle between split-row and paged modes
- **Visual feedback**: Brief toast "SPLIT MODE" or "PAGED 1/2"
- **Persistent**: Remember per-project or global

### Paged Mode Navigation
- **Auto-paging**: Cursor automatically switches banks
  - Channels 0-3: Shows bank 1 (channels 1-4)
  - Channels 4-7: Shows bank 2 (channels 5-8)
- **Manual override**: L + Left/Right to switch banks manually
  - Useful for viewing other channels while editing

**Note**: L used as modifier to avoid conflicts with RT (view navigation) commands

## Implementation Priority

### Phase 1: Split-Row Only (3-4 days)
Implement two-row display as default for RG Nano

### Phase 2: Add Paging (1-2 days)
Add paged mode as alternative

### Phase 3: Toggle System (1 day)
Add runtime toggle and config

### Phase 4: Smart Defaults (1 day)
Context-aware mode selection

## Conclusion

**Best Approach**: Implement BOTH, let users toggle

**Your instinct is RIGHT** - split-row mode is actually better for most tracker workflows because:
1. Context is king in music composition
2. Vertical space is abundant (square screen)
3. Seeing all channels > seeing more rows

**But offer paging for**:
- Users who want traditional layout
- Simpler songs (4 channels or less)
- Situations where maximum row visibility matters

**Default recommendation**: Split-row mode with clear visual separation and SELECT+L quick toggle.