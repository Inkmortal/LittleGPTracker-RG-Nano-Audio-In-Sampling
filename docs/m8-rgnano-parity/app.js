const M8_MANUAL_URL = "https://cdn.shopify.com/s/files/1/0455/0485/6229/files/m8_operation_manual_v20260421.pdf?v=1776791699";
const ASSET_VERSION = "20260503-legend-v1";

const screens = {
  song: { name: "Song", status: "implemented", ref: "M8 manual p10 / RG sim", summary: "The big timeline. Each row is a moment in the song; each column is one track/channel; each cell says which chain that track should play.", columns: ["row = time/section", "column = track/channel", "cell = chain number"], actions: ["M8 SHIFT+RIGHT -> Chain", "RG R+Right -> Chain"], capture: "assets/rg-song-chain.png", rows: ["00 00 -- -- -- -- -- --", "01 01 -- 02 -- -- -- --", "02 03 04 -- -- -- -- --"] },
  live: { name: "Live", status: "partial", ref: "M8 p10 / RG input map", summary: "Cue chains independently for performance. M8 has a clear mode; RG live mode needs audit.", columns: ["track launch state", "queued chains"], actions: ["RG B+Left/Right toggles live/song"], rows: ["TRK 1 2 3 4 5 6 7 8", "CUE 00 -- 02 -- -- -- -- --"] },
  chain: { name: "Chain", status: "implemented", ref: "M8 p12 / RG input map", summary: "A mini playlist for one track. A Song cell points to a Chain; the Chain lists which Phrases play in order.", columns: ["chain row = order", "phrase number = pattern to play", "transpose = pitch shift"], actions: ["M8/RG right -> Phrase"], capture: "assets/rg-chain-phrase.png", rows: ["00 PH 00 TSP 00", "01 PH 01 TSP 00", "02 PH 02 TSP 00"] },
  phrase: { name: "Phrase", status: "implemented", ref: "M8 p14 / RG sim", summary: "The note pattern. Each phrase row is a small step where you can play a note, choose an instrument, and add command effects.", columns: ["row = step", "note = pitch/rest", "instrument = sound slot", "command/value = effect"], actions: ["A+D-pad edits", "L+D-pad chromatic override on RG"], capture: "assets/rg-phrase-note.png", rows: ["00 C-2 00 VOL 80", "01 --- -- --- --", "02 G-2 02 TBL 01"] },
  instrument: { name: "Instrument", status: "partial", ref: "M8 p16 / RG input map", summary: "Sound design. RG has sample instruments; M8 has multiple engines.", columns: ["engine/type", "sample", "envelope", "filter", "table"], actions: ["RG A on sample opens import"], capture: "assets/rg-instrument.png", rows: ["TYPE SAMPLER", "SAMPLE wuxia", "FILTER LP 64"] },
  modulation: { name: "Modulation", status: "missing", ref: "M8 p18", summary: "M8 LFO/envelope modulation. Not in RG Nano yet.", columns: ["not present on RG"], actions: ["Use LGPT tables for now"], rows: ["MOD1 LFO -> CUT", "MOD2 ENV -> AMP"] },
  pool: { name: "Instrument Pool", status: "missing", ref: "M8 p22", summary: "Dedicated instrument browser/pool. Not in RG Nano yet.", columns: ["not present on RG"], actions: ["Needs compact browser"], rows: ["00 Kick", "01 Snare", "02 Guzheng"] },
  table: { name: "Table", status: "implemented", ref: "M8 p24 / RG input map", summary: "Command sequencer for motion and effects.", columns: ["row", "command", "value"], actions: ["A+Up/Down on command opens selector"], capture: "assets/rg-table-phrase.png", rows: ["00 PIT 0004", "01 VOL 0070", "02 FLT 0038"] },
  groove: { name: "Groove", status: "implemented", ref: "M8 p26 / RG input map", summary: "Timing values and swing feel.", columns: ["step", "ticks"], actions: ["RG R+Down returns to Phrase"], capture: "assets/rg-groove.png", rows: ["00 06", "01 06", "02 08"] },
  scale: { name: "Scale", status: "missing", ref: "M8 p28", summary: "Dedicated scale editor. RG has Project Key/Scale but no dedicated screen.", columns: ["project Key", "project Scale"], actions: ["RG uses Project fields"], rows: ["C D E G A", "KEY --"] },
  mixer: { name: "Mixer", status: "implemented", ref: "M8 p30 / RG sim", summary: "Levels and activity. RG has channel scopes and master waveform.", columns: ["channels", "levels", "scopes"], actions: ["RG R+Up -> Song"], capture: "assets/rg-mixer-live.png", rows: ["00 01 02 03 04 05 06 07", "LV 78 70 64 58 66 52 48 60"] },
  eq: { name: "EQ / Limiter", status: "missing", ref: "M8 p32-34", summary: "Tone/dynamics control. Not in RG Nano yet.", columns: ["not present on RG"], actions: ["Needs design/implementation"], rows: ["LOW -2", "MID +1", "LIMIT .85"] },
  fx: { name: "Effect Settings", status: "partial", ref: "M8 p36", summary: "Global delay/reverb/mod FX. RG command/effect path needs audit.", columns: ["FX type", "time", "feedback", "mix"], actions: ["Needs RG audit"], rows: ["DLY TIME 32", "REV MIX 18"] },
  project: { name: "Project", status: "implemented", ref: "M8 p38 / RG input map", summary: "Tempo, save/load, key, scale, render fields.", columns: ["tempo", "key", "scale", "save/render"], actions: ["RG R+Down -> Song"], capture: "assets/rg-project.png", rows: ["TEMPO 100", "KEY --", "SCALE CHROM"] },
  system: { name: "System", status: "partial", ref: "M8 p40", summary: "Hardware/app settings. RG has config/platform settings, not a polished producer screen.", columns: ["settings"], actions: ["Low priority"], rows: ["AUDIO", "DISPLAY", "INPUT"] },
  theme: { name: "Theme", status: "missing", ref: "M8 p42", summary: "Theme editor. Not in RG Nano yet.", columns: ["not present"], actions: ["Low priority"], rows: ["COLOR 01", "COLOR 02"] },
  midi: { name: "MIDI", status: "partial", ref: "M8 p43-44 / LGPT MIDI", summary: "MIDI settings/mapping. Existing LGPT support needs RG Nano hardware audit.", columns: ["clock", "in/out", "mapping"], actions: ["Needs hardware proof"], rows: ["CLOCK OFF", "OUT CH 01"] },
  render: { name: "Render", status: "partial", ref: "M8 p47 / RG project fields", summary: "Bounce audio. RG Project render now exports native stereo and stem WAVs; M8 still has deeper range/name/render-to-sample controls.", columns: ["mode", "output", "missing parity"], actions: ["RG Project Render -> Stereo/Stems", "Start begins render", "Start stops/closes WAV"], capture: "assets/rg-project.png", rows: ["STEREO mixdown.wav", "STEMS channel0.wav"] },
  command: { name: "Command Help", status: "partial", ref: "M8 p48 / local code", summary: "Command picker/help. RG has command selector in command columns but needs a clear tutorial/test.", columns: ["command", "description", "value"], actions: ["A+Up/Down on command column"], rows: ["VOL volume", "PIT pitch", "TBL table"] },
  sampler: { name: "Sampler / Editor", status: "partial", ref: "M8 p54-56 / RG import", summary: "RG imports/previews WAVs. Deep crop/normalize/slice editor is missing.", columns: ["sample", "start", "loop", "slice"], actions: ["RG A on Import assigns sample"], capture: "assets/rg-sample-import.png", rows: ["LISTEN", "IMPORT", "SLICE --"] },
  boot: { name: "Project Select", status: "implemented", ref: "RG sim", summary: "Load/new/exit boot modal.", columns: ["project list", "Load/New/Exit"], actions: ["A activates"], capture: "assets/rg-project-select.png", rows: ["LOAD", "NEW", "EXIT"] },
  power: { name: "Power Menu", status: "implemented", ref: "RG app", summary: "Menu/Power opens the app exit/debug overlay; FN/Select remains the tracker Select key.", columns: ["exit/debug actions"], actions: ["Menu/Power"], capture: "assets/rg-power-menu.png", rows: ["EXIT", "DEBUG"] }
};

const m8ManualRefs = {
  song: { label: "Song View", page: 10 },
  live: { label: "Live Mode", page: 10 },
  chain: { label: "Chain View", page: 12 },
  phrase: { label: "Phrase View", page: 14 },
  instrument: { label: "Instrument View", page: 16 },
  modulation: { label: "Instrument Modulation View", page: 18 },
  pool: { label: "Instrument Pool View", page: 22 },
  table: { label: "Table View", page: 24 },
  groove: { label: "Groove View", page: 26 },
  scale: { label: "Scale View", page: 28 },
  mixer: { label: "Mixer View", page: 30 },
  eq: { label: "EQ Editor / Limiter & Mix Scope Views", page: 32, endPage: 34 },
  fx: { label: "Effect Settings View", page: 36 },
  project: { label: "Project View", page: 38 },
  system: { label: "System Settings View", page: 40 },
  theme: { label: "Theme View", page: 42 },
  midi: { label: "MIDI Mapping / MIDI Settings Views", page: 43, endPage: 44 },
  render: { label: "Render View", page: 47 },
  command: { label: "Effect Command Help View", page: 48 },
  sampler: { label: "Sampler / Sample Editor", page: 54, endPage: 56 }
};

const m8ManualCrops = {
  song: "assets/m8-manual/m8-song-rendered-p10.png",
  live: "assets/m8-manual/m8-live-rendered-p10.png",
  chain: "assets/m8-manual/m8-chain-rendered-p12.png",
  phrase: "assets/m8-manual/m8-phrase-rendered-p14.png",
  instrument: "assets/m8-manual/m8-instrument-rendered-p16.png",
  modulation: "assets/m8-manual/m8-modulation-rendered-p18.png",
  pool: "assets/m8-manual/m8-pool-rendered-p22.png",
  table: "assets/m8-manual/m8-table-rendered-p24.png",
  groove: "assets/m8-manual/m8-groove-rendered-p26.png",
  scale: "assets/m8-manual/m8-scale-rendered-p28.png",
  mixer: "assets/m8-manual/m8-mixer-rendered-p30.png",
  eq: "assets/m8-manual/m8-eq-rendered-p32.png",
  fx: "assets/m8-manual/m8-fx-rendered-p36.png",
  project: "assets/m8-manual/m8-project-rendered-p38.png",
  system: "assets/m8-manual/m8-system-rendered-p40.png",
  theme: "assets/m8-manual/m8-theme-rendered-p42.png",
  midi: "assets/m8-manual/m8-midi-rendered-p44.png",
  render: "assets/m8-manual/m8-render-rendered-p47.png",
  command: "assets/m8-manual/m8-command-rendered-p48.png",
  sampler: "assets/m8-manual/m8-sampler-rendered-p54.png"
};

function manualPdfPage(manualPage) {
  return manualPage + 7;
}

function manualPageText(ref) {
  if (!ref) return "";
  return ref.endPage ? `manual pp. ${ref.page}-${ref.endPage}` : `manual p. ${ref.page}`;
}

function manualLink(screenId) {
  const ref = m8ManualRefs[screenId];
  if (!ref) return "";
  const href = `${M8_MANUAL_URL}#page=${manualPdfPage(ref.page)}`;
  return `<a href="${href}" target="_blank" rel="noreferrer">${ref.label}, ${manualPageText(ref)}</a>`;
}

const graphMaps = {
  m8: {
    title: "Dirtywave M8",
    nodes: [
      ["song", 8, 8], ["chain", 27, 8], ["phrase", 46, 8], ["instrument", 65, 8], ["modulation", 84, 8],
      ["live", 8, 31], ["table", 46, 31], ["groove", 27, 31], ["pool", 65, 31], ["sampler", 84, 31],
      ["mixer", 8, 56], ["eq", 27, 56], ["fx", 46, 56], ["project", 65, 56], ["render", 84, 56],
      ["scale", 65, 79], ["system", 46, 79], ["theme", 27, 79], ["midi", 8, 79], ["command", 84, 79]
    ],
    edges: [["song","chain"],["chain","phrase"],["phrase","instrument"],["instrument","modulation"],["song","live"],["phrase","table"],["chain","groove"],["instrument","pool"],["instrument","sampler"],["song","mixer"],["mixer","eq"],["mixer","fx"],["song","project"],["project","render"],["project","scale"],["project","system"],["project","theme"],["project","midi"],["phrase","command"]]
  },
  rg: {
    title: "RG Nano LGPT",
    nodes: [
      ["boot", 8, 8], ["project", 27, 8], ["song", 46, 8], ["chain", 65, 8], ["phrase", 84, 8],
      ["live", 46, 31], ["mixer", 27, 31], ["instrument", 84, 31], ["sampler", 65, 31], ["command", 84, 54],
      ["table", 65, 54], ["groove", 46, 54], ["power", 8, 54], ["render", 27, 54],
      ["modulation", 8, 79], ["pool", 27, 79], ["scale", 46, 79], ["eq", 65, 79], ["fx", 84, 79]
    ],
    edges: [["boot","project"],["project","song"],["song","chain"],["chain","phrase"],["song","live"],["song","mixer"],["phrase","instrument"],["instrument","sampler"],["phrase","table"],["phrase","groove"],["phrase","command"],["project","render"],["power","song"]]
  }
};

const m8Board = {
  width: 1860,
  height: 1120,
  groups: [
    { label: "Compose", x: 40, y: 60, w: 705, h: 335, tone: "pink" },
    { label: "Design Sound", x: 810, y: 60, w: 820, h: 365, tone: "cyan" },
    { label: "Motion", x: 445, y: 500, w: 620, h: 440, tone: "amber" },
    { label: "Mix", x: 40, y: 710, w: 560, h: 315, tone: "green" },
    { label: "Finish / System", x: 1130, y: 540, w: 610, h: 440, tone: "blue" }
  ],
  nodes: {
    song: { x: 88, y: 125, w: 220, h: 178, size: "large", note: "arrange" },
    chain: { x: 342, y: 132, w: 200, h: 162, note: "sections" },
    phrase: { x: 585, y: 112, w: 240, h: 198, size: "large", note: "notes" },
    live: { x: 92, y: 320, w: 200, h: 154, note: "perform" },

    instrument: { x: 890, y: 112, w: 240, h: 198, size: "large", note: "engine" },
    modulation: { x: 1170, y: 116, w: 210, h: 166, note: "lfo/env" },
    pool: { x: 1415, y: 112, w: 210, h: 166, note: "manage" },
    sampler: { x: 1200, y: 320, w: 245, h: 172, size: "wide", note: "record/edit" },

    table: { x: 505, y: 570, w: 215, h: 158, note: "commands" },
    groove: { x: 775, y: 570, w: 200, h: 152, note: "swing" },
    command: { x: 775, y: 770, w: 220, h: 152, note: "fx help" },

    mixer: { x: 100, y: 800, w: 225, h: 178, size: "large", note: "levels" },
    eq: { x: 360, y: 765, w: 205, h: 152, note: "tone" },
    fx: { x: 360, y: 945, w: 215, h: 144, note: "send fx" },

    project: { x: 1195, y: 620, w: 240, h: 190, size: "large", note: "project hub" },
    scale: { x: 1485, y: 575, w: 205, h: 152, note: "key/scale" },
    render: { x: 1485, y: 775, w: 215, h: 158, note: "bounce" },
    system: { x: 1190, y: 890, w: 200, h: 140, note: "settings" },
    midi: { x: 930, y: 970, w: 200, h: 140, note: "sync/io" },
    theme: { x: 1605, y: 970, w: 185, h: 128, note: "colors" }
  },
  edges: [
    ["song", "chain"], ["chain", "phrase"], ["song", "live"],
    ["phrase", "instrument"], ["instrument", "modulation"], ["instrument", "pool"], ["instrument", "sampler"],
    ["phrase", "table"], ["chain", "groove"], ["phrase", "command"],
    ["song", "mixer"], ["mixer", "eq"], ["mixer", "fx"],
    ["song", "project"], ["project", "scale"], ["project", "render"], ["project", "system"], ["project", "midi"], ["project", "theme"]
  ]
};

const rgBoard = {
  width: 1320,
  height: 840,
  groups: [
    { label: "Project Start", x: 34, y: 58, w: 330, h: 240, tone: "blue" },
    { label: "Compose", x: 404, y: 58, w: 500, h: 270, tone: "pink" },
    { label: "Sound + Motion", x: 470, y: 396, w: 520, h: 290, tone: "cyan" },
    { label: "Monitor + Finish", x: 34, y: 390, w: 380, h: 290, tone: "green" },
    { label: "M8 Parity Gaps", x: 1040, y: 70, w: 238, h: 700, tone: "amber" }
  ],
  nodes: {
    boot: { x: 72, y: 118, w: 132, h: 162, note: "select" },
    project: { x: 224, y: 118, w: 132, h: 162, note: "tempo/render" },
    song: { x: 438, y: 108, w: 150, h: 184, size: "large", note: "arrange" },
    chain: { x: 612, y: 112, w: 142, h: 176, note: "sections" },
    phrase: { x: 782, y: 100, w: 154, h: 192, size: "large", note: "notes" },
    live: { x: 442, y: 302, w: 132, h: 122, note: "needs audit" },

    instrument: { x: 802, y: 454, w: 150, h: 184, note: "sample params" },
    sampler: { x: 624, y: 454, w: 150, h: 184, note: "import" },
    table: { x: 624, y: 658, w: 142, h: 162, note: "commands" },
    groove: { x: 472, y: 644, w: 132, h: 158, note: "timing" },
    command: { x: 812, y: 660, w: 142, h: 132, note: "needs audit" },

    mixer: { x: 82, y: 456, w: 150, h: 184, size: "large", note: "monitor" },
    render: { x: 252, y: 456, w: 132, h: 162, note: "partial" },
    power: { x: 252, y: 640, w: 122, h: 116, note: "global" },

    modulation: { x: 1072, y: 122, w: 150, h: 112, note: "not in RG" },
    pool: { x: 1072, y: 258, w: 150, h: 112, note: "not in RG" },
    scale: { x: 1072, y: 394, w: 150, h: 112, note: "project field only" },
    eq: { x: 1072, y: 530, w: 150, h: 112, note: "not in RG" },
    fx: { x: 1072, y: 666, w: 150, h: 112, note: "needs audit" }
  },
  edges: [
    ["boot", "project"], ["project", "song"], ["song", "chain"], ["chain", "phrase"], ["song", "live"],
    ["phrase", "instrument"], ["instrument", "sampler"], ["phrase", "table"], ["phrase", "groove"], ["phrase", "command"],
    ["song", "mixer"], ["project", "render"], ["power", "song"],
    ["instrument", "modulation"], ["instrument", "pool"], ["project", "scale"], ["mixer", "eq"], ["mixer", "fx"]
  ]
};

const m8Thumbs = {
  song: { kind: "grid", meta: "SONG", rows: ["00 00 -- -- -- -- -- --", "01 01 02 -- -- -- -- --", "02 -- -- 03 04 -- -- --", "03 05 05 06 -- -- -- --", "04 -- -- -- 07 08 -- --"] },
  live: { kind: "grid", meta: "LIVE", rows: ["TRK 1  2  3  4  5  6  7  8", "CUE 00 -- 02 -- -- -- -- --", "MUT .. .. .. .. .. .. .. ..", "RUN >  .  .  .  .  .  .  ."] },
  chain: { kind: "grid", meta: "CHAIN", rows: ["00 PHRASE 00 TRANS 00", "01 PHRASE 01 TRANS 00", "02 PHRASE 02 TRANS +12", "03 PHRASE -- TRANS 00", "04 PHRASE 03 TRANS -12"] },
  phrase: { kind: "grid", meta: "PHRASE", rows: ["00 C-3 01 80 TBL 01", "01 --- -- -- DLY 24", "02 G-3 02 78 CUT 44", "03 A#3 02 70 REV 18", "04 --- -- -- HOP 00"] },
  instrument: { kind: "params", meta: "INSTR", params: ["ENGINE WAVSYN", "VOL 80", "ENV A 02", "ENV R 18", "FILTER LP", "TABLE 01"] },
  modulation: { kind: "params", meta: "MOD", params: ["LFO1 TRI", "AMT +24", "DEST CUTOFF", "ENV AMP", "TRACK NOTE", "SPEED 12"] },
  pool: { kind: "grid", meta: "POOL", rows: ["00 KICK.WAV", "01 SNARE.WAV", "02 GUZHENG.WAV", "03 BASS.WAV", "04 PAD.WAV"] },
  sampler: { kind: "wave", meta: "SAMPLE", rows: ["START 000000", "END   028800", "LOOP  001800", "SLICE 08"] },
  table: { kind: "grid", meta: "TABLE", rows: ["00 VOL 0080", "01 PIT 0004", "02 CUT 0038", "03 HOP 0000", "04 REP 0002"] },
  groove: { kind: "grid", meta: "GROOVE", rows: ["00 06", "01 06", "02 08", "03 04", "04 06", "05 07"] },
  command: { kind: "grid", meta: "CMD", rows: ["VOL volume", "PIT pitch", "TBL table", "DLY delay", "HOP jump"] },
  mixer: { kind: "bars", meta: "levels", bars: [62, 44, 76, 34, 58, 42, 68, 52] },
  eq: { kind: "params", meta: "MIX", params: ["TRACK 01", "LOW -2", "MID +1", "HIGH +0", "LIMIT 82", "GAIN 90"] },
  fx: { kind: "params", meta: "FX", params: ["DLY TIME 32", "DLY FB 42", "REV SIZE 70", "REV MIX 22", "CHOR 14", "SEND 03"] },
  project: { kind: "params", meta: "PROJ", params: ["TEMPO 086", "TRANS 00", "SAVE", "LOAD", "RENDER", "SYSTEM"] },
  scale: { kind: "grid", meta: "SCALE", rows: ["ROOT D", "MODE MINOR", "D  E  F  G  A  Bb C", "ON ON ON ON ON ON ON"] },
  render: { kind: "params", meta: "RENDER", params: ["SOURCE SONG", "RANGE ALL", "FORMAT WAV", "DEPTH 24", "NORMALIZE", "START"] },
  system: { kind: "params", meta: "SYSTEM", params: ["AUDIO", "DISPLAY", "MIDI", "CLOCK", "INPUT", "FILES"] },
  midi: { kind: "params", meta: "MIDI", params: ["CLOCK OUT", "CH 01", "CC MAP", "SYNC", "USB", "TRS"] },
  theme: { kind: "params", meta: "THEME", params: ["BACKGROUND", "TEXT", "HIGHLIGHT", "CURSOR", "SCOPE", "SAVE"] }
};

const flows = [
  { title: "Build first pattern", summary: "Make one repeating idea: Song cell -> Chain -> Phrase", steps: [
    ["Pick a track and row", "Choose where one repeating part will start.", "song", "song", "M8 EDIT / SHIFT+RIGHT", "RG A / R+Right"],
    ["Make a chain", "A chain is the list of phrase patterns this one track will play.", "chain", "chain", "M8 SHIFT+RIGHT", "RG R+Right"],
    ["Write the phrase", "A phrase is the actual note pattern, like a drum beat, bass riff, or melody.", "phrase", "phrase", "M8 EDIT+D-pad", "RG A+D-pad"],
    ["Loop and listen", "Play it back while watching which row/track is active.", "phrase", "phrase", "M8 PLAY", "RG Start"]
  ]},
  { title: "Build instruments", summary: "Instrument -> sampler -> modulation", steps: [
    ["Choose sound slot", "M8 chooses engines; RG currently edits sample instruments.", "instrument", "instrument", "M8 Instrument", "RG Instrument"],
    ["Load samples", "RG can preview and import uploaded WAVs.", "sampler", "sampler", "M8 Sampler", "RG Sample Import"],
    ["Add motion", "M8 modulation is not in RG yet; use tables for now.", "modulation", "table", "M8 Modulation", "RG Table"]
  ]},
  { title: "Arrange full song", summary: "Spread patterns and variations across time", steps: [
    ["Clone variations", "Copy the first pattern and change small things for fills, endings, or new sections.", "chain", "chain", "M8 copy/clone", "RG B+L then A+L"],
    ["Place sections", "Put different chain numbers on later Song rows so the music changes over time.", "song", "song", "M8 Song", "RG Song"],
    ["Add feel", "Use groove and tables after the parts exist to make the rhythm less stiff.", "groove", "groove", "M8 Groove", "RG Groove"]
  ]},
  { title: "Mix and monitor", summary: "Mixer, scopes, FX, EQ, limiter", steps: [
    ["Open mixer", "Watch track activity and levels.", "mixer", "mixer", "M8 Mixer", "RG R+Down"],
    ["Shape tone", "EQ/limiter are M8 features; RG does not have dedicated screens yet.", "eq", "eq", "M8 EQ/Limiter", "Not in RG yet"],
    ["Set effects", "Send FX workflow needs RG audit.", "fx", "fx", "M8 FX", "RG partial/unknown"]
  ]},
  { title: "Render a track", summary: "Bounce final audio", steps: [
    ["Open project", "Save and set project defaults.", "project", "project", "M8 Project", "RG Project"],
    ["Render WAV", "RG needs a first-class render route and test.", "render", "render", "M8 Render", "RG partial"],
    ["Resample", "Selection-to-sample is not in RG yet.", "sampler", "sampler", "M8 Selection to Sample", "RG missing editor"]
  ]}
];

const screenManuals = {
  m8: {
    song: {
      purpose: "Song View is the top-level timeline. Read it like a spreadsheet: rows go downward through time, columns go left-to-right across the 8 tracks, and each cell contains the chain number that track should play at that song row.",
      workflow: "To start a loop, pick row 00 and a track column, enter chain 00, then open that chain. To arrange a longer song, place different chain numbers on later rows so the music changes section by section.",
      navigation: "Open Chain View from the selected Song cell to edit the thing that cell points at. Live Mode cues chains for performance, Mixer checks levels, and Project holds tempo/render/settings.",
      fields: ["T128 = tempo 128 BPM", "row number = position in the song", "track columns 1-8 = the 8 channels", "00-FE = chain numbers", "-- = nothing plays in that cell"],
      tips: ["A column does not play left or right; it is one vertical track over time.", "Rows matter for when things happen. Columns matter for which track plays them."]
    },
    live: {
      purpose: "Live Mode is for cueing, stopping, and remixing chains independently while the song is running.",
      workflow: "Cue a chain on one or more tracks, then let the live quantize setting decide when the change begins.",
      navigation: "Enter/exit from Song View. Think of this as a performance layer over the same song/chain material.",
      fields: ["track cue state", "queued chain", "running position"],
      tips: ["Use Live Mode to test arrangement ideas before committing them into Song View."]
    },
    chain: {
      purpose: "Chain View explains what a Song cell actually plays. A chain is a short ordered list of phrases for one track, like 'play phrase 00, then phrase 01, then phrase 02.'",
      workflow: "Create a chain row, put a phrase number there, then open that phrase to write notes. Add more chain rows when that track should play multiple phrase patterns in sequence.",
      navigation: "Song points to Chain; Chain points to Phrase. Chain is the middle layer between arrangement and note entry.",
      fields: ["chain row = order inside this chain", "phrase number = which note pattern to play", "transpose = optional pitch shift for that phrase"],
      tips: ["If Song is the big timeline, Chain is a small playlist inside one cell.", "One chain belongs to one track at the moment it is used; other tracks can use their own chains at the same song row."]
    },
    phrase: {
      purpose: "Phrase View is where sound events are written. Each row is one step in the pattern. A row can say 'play note C-3 with instrument 01 at this moment' and can also include command effects.",
      workflow: "Move down to choose the step in time, move sideways to choose what part of the event you are editing: note, instrument, volume/velocity, or command/value.",
      navigation: "From Phrase, jump to Instrument when the sound itself needs editing, Table for repeated command motion, Groove for timing feel, or Command Help when you do not remember a command.",
      fields: ["note = pitch or rest", "instrument = which sound slot plays", "velocity/volume = loudness", "command/value columns = tracker effects such as table, pitch, volume, delay"],
      tips: ["Phrase rows are not separate tracks; they are steps in time for the current track.", "A single phrase track is monophonic, so chords usually use multiple tracks or pre-made chord samples."]
    },
    instrument: {
      purpose: "Instrument View chooses the sound engine and edits the sound assigned to an instrument slot.",
      workflow: "Pick an engine, set volume/filter/envelope/send values, then jump into modulation, sample editor, or pool management when needed.",
      navigation: "Usually entered from Phrase. Instrument Pool manages slots; Sampler/Sample Editor handle sample sources; Modulation adds movement.",
      fields: ["engine type", "sample/oscillator settings", "filter", "envelope", "sends"],
      tips: ["M8 supports multiple native engines. RG Nano currently does not."]
    },
    modulation: {
      purpose: "Instrument Modulation routes LFOs/envelopes/tracking sources into instrument parameters.",
      workflow: "Choose a modulation source, destination, amount, and timing so an instrument moves without writing command data on every phrase row.",
      navigation: "Entered from Instrument View.",
      fields: ["mod source", "destination", "amount", "speed/shape"],
      tips: ["This is a major M8 sound-design gap for RG Nano."]
    },
    pool: {
      purpose: "Instrument Pool is the instrument management screen: browse, copy, organize, and reuse instrument slots.",
      workflow: "Use it when a project grows beyond a few sounds and you need to duplicate, replace, or clean up instruments.",
      navigation: "Entered from Instrument View.",
      fields: ["instrument number", "name/type", "copy/manage actions"],
      tips: ["RG Nano needs a compact equivalent because sample-heavy songs can get messy quickly."]
    },
    table: {
      purpose: "Table View is a small command sequencer used for repeating motion, arps, retrigs, pitch moves, and parameter changes.",
      workflow: "Write command/value rows, then trigger the table from phrases or attach it to an instrument.",
      navigation: "Entered from Phrase or Instrument depending on the table context.",
      fields: ["row", "command", "value"],
      tips: ["Tables are one of the best ways to get M8-like motion on RG Nano today."]
    },
    groove: {
      purpose: "Groove View changes timing by setting tick lengths for groove steps.",
      workflow: "Edit tick values to create swing or uneven timing, then assign/use that groove in the sequence.",
      navigation: "Reached from the composition area, commonly after phrase/chain writing.",
      fields: ["groove row", "tick count"],
      tips: ["Small timing changes can make simple loops feel much less rigid."]
    },
    scale: {
      purpose: "Scale View defines allowed notes so melodic entry can follow a key or mode.",
      workflow: "Set root/mode or enabled notes, then return to phrase writing with guided note steps.",
      navigation: "M8 has a dedicated Scale View; RG Nano currently uses Project fields.",
      fields: ["root", "scale/mode", "enabled notes"],
      tips: ["A tiny dedicated RG scale page is a high-value future screen."]
    },
    mixer: {
      purpose: "Mixer View balances track, input, send, and output levels while showing meters.",
      workflow: "Watch which tracks are active, adjust volumes/sends, and check overall level before rendering.",
      navigation: "Accessible from the song/performance area; deeper mix pages lead to EQ, limiter/scope, and FX settings.",
      fields: ["track levels", "speaker/headphone volume", "send levels", "meter bars"],
      tips: ["The meter colors in this site are preserved from the extracted manual render."]
    },
    eq: {
      purpose: "EQ Editor and Limiter/Mix Scope shape tone and control output loudness.",
      workflow: "Use EQ per track/input/master, then check limiter/scope behavior so the render is controlled.",
      navigation: "M8 reaches these from the mixer layer.",
      fields: ["EQ bands", "limiter settings", "scope/mix display"],
      tips: ["RG Nano has scope visuals but not an equivalent EQ/limiter workflow yet."]
    },
    fx: {
      purpose: "Effect Settings configures global send effects such as delay, reverb, chorus, and modulation effects.",
      workflow: "Set global effect character here, then send tracks/instruments into those effects from instrument or mixer parameters.",
      navigation: "Near Mixer in the M8 graph.",
      fields: ["delay", "reverb", "chorus/mod FX", "send behavior"],
      tips: ["RG Nano needs an audit of existing commands/effects before adding a new page."]
    },
    project: {
      purpose: "Project View holds song-wide settings: tempo, saving/loading, rendering, and system routes.",
      workflow: "Use it when the decision affects the whole project rather than a single phrase, chain, or instrument.",
      navigation: "Acts as the hub for render, scale, system, theme, and MIDI settings.",
      fields: ["tempo", "save/load", "render", "system links"],
      tips: ["On RG Nano this is where key/scale/note spelling currently live."]
    },
    system: {
      purpose: "System Settings controls the hardware-facing behavior of the M8: display, audio, MIDI/sync, input, battery, and file behavior.",
      workflow: "Use it when a project problem is really a device problem: clocking, audio output, controller behavior, or UI preferences.",
      navigation: "Reached from Project View. It is not part of the main writing loop, but it matters for a finished live or portable setup.",
      fields: ["audio", "display", "input", "sync/MIDI", "storage"],
      tips: ["RG Nano needs a compact equivalent only for settings that affect music production directly."]
    },
    theme: {
      purpose: "Theme View edits the visual palette of the M8 interface.",
      workflow: "Change colors when readability, stage lighting, or personal preference makes the default palette less useful.",
      navigation: "Reached from Project/System routes rather than the core song-writing path.",
      fields: ["background", "text", "highlight", "cursor", "meter/accent"],
      tips: ["This is a low-priority RG parity target compared with sampler, render, mixer, and synth engines."]
    },
    midi: {
      purpose: "MIDI Mapping and MIDI Settings connect the M8 to controllers, clocks, external instruments, and synchronization workflows.",
      workflow: "Map controls, set channels, choose clock behavior, then return to the song or live performance surface.",
      navigation: "Reached from Project/System settings; used before writing or performing with external gear.",
      fields: ["clock", "input", "output", "channels", "CC mappings"],
      tips: ["RG Nano MIDI parity depends on the actual hardware route available on the handheld."]
    },
    render: {
      purpose: "Render View bounces audio to WAV, including mixed output or stems depending on settings.",
      workflow: "Set row range, repeat count, track selection, effects inclusion, output mode, and name, then render.",
      navigation: "Entered from Project View.",
      fields: ["song row start/last", "tracks", "mixed/stems", "bit depth/name"],
      tips: ["This is a P1 RG Nano parity gap because a portable studio has to finish audio."]
    },
    command: {
      purpose: "Effect Command Help explains command names and lets the user choose commands in context.",
      workflow: "Open it from an FX command column, read the short description, and insert the selected command.",
      navigation: "Contextual from Phrase or Table command columns.",
      fields: ["command category", "command abbreviation", "description"],
      tips: ["RG Nano needs a confirmed universal/contextual shortcut for this."]
    },
    sampler: {
      purpose: "Sampler and Sample Editor load, record, crop, slice, loop, and process sample material.",
      workflow: "Load or record a sample, tune it, set loop/slice points, and use offline edits such as crop/normalize/slice when needed.",
      navigation: "Entered from Instrument/Sampler workflows.",
      fields: ["sample name", "slice/play/start/loop/length", "normalize/crop/slice tools"],
      tips: ["This is central for RG Nano because uploaded samples replace live mic sampling."]
    }
  },
  rg: {
    boot: {
      purpose: "The project selector is the RG Nano session start screen.",
      workflow: "Choose Load to open an existing project, New to create one, or Exit to leave the app.",
      navigation: "Use D-pad to move selection and A to activate.",
      fields: ["Load", "New", "Exit"],
      tips: ["This screen is already captured at real 240x240."]
    },
    song: {
      purpose: "Song View is the RG Nano timeline. Rows go downward through time; the 8 columns are the 8 audio tracks/channels; each cell contains the chain number that track should play.",
      workflow: "For a loop, start at row 00, choose a channel column, and enter a chain number. For a full arrangement, put later chain numbers on later rows so sections change over time.",
      navigation: "R+Right opens the Chain for the selected cell. R+Down opens Mixer. R+Up opens Project. R+Select opens the helper/minimap. The bottom strip is visual feedback, not extra song data.",
      fields: ["row = song position", "columns 00-07 = channels/tracks", "cell value = chain number", "-- = empty/no chain", "PLAY label = current playback scope", "bottom boxes = channel activity/level feedback"],
      tips: ["All 8 track columns fit on RG Nano, so there is no horizontal paging on Song View.", "Rows scroll vertically through the 256-row song.", "Start should show PLAY:SONG when the arrangement is running."]
    },
    chain: {
      purpose: "Chain View shows the phrase playlist for the selected Song cell. It does not contain notes directly; it contains phrase numbers.",
      workflow: "Put phrase 00 in chain row 00, then open Phrase View to write phrase 00. Add phrase 01 on the next chain row if this track should play a second pattern afterward.",
      navigation: "R+Left returns to Song; R+Right opens the selected Phrase. Start plays current-channel chain context; R+Start plays song context. R+Select opens the helper/minimap.",
      fields: ["chain row = order", "phrase = note pattern number", "transpose = pitch shift"],
      tips: ["Chain is the bridge: Song chooses a chain, Chain chooses phrases, Phrase contains notes.", "Use separate chains for variations like drum fill, bass change, or melody answer.", "Check the PLAY label: PLAY:CHAIN means focused chain playback, PLAY:SONG means arrangement context."]
    },
    live: {
      purpose: "Live Mode is the RG performance/cueing target for trying chains without rewriting the song arrangement.",
      workflow: "Cue chains while playback runs, listen for arrangement ideas, then commit the best choices back into Song View.",
      navigation: "The current map routes it from Song. The exact live-screen shortcut and quantized cue behavior still need device-level confirmation.",
      fields: ["track", "queued chain", "running state"],
      tips: ["Label this partial until the simulator and hardware prove the full live workflow."]
    },
    phrase: {
      purpose: "Phrase View is the RG Nano note editor. Each row is a step in the pattern. The row can contain a note, the instrument slot that plays it, and command/value pairs for effects.",
      workflow: "Move up/down through the 16 time steps. Move left/right through fields. A+D-pad edits normally; L+D-pad gives a quick chromatic note override when scale guidance is active.",
      navigation: "R+Left returns Chain; R+Right opens Instrument; R+Down opens Table; R+Up opens Groove. Select on command columns opens the command picker. Start plays current-channel phrase context; R+Start plays song context.",
      fields: ["note = pitch/rest", "instrument = sound slot", "command/value pairs = tracker effects", "PLAY label = song/phrase/audition scope", "mini waveform/activity strip = playback feedback"],
      tips: ["RG is mostly at parity for core note entry: notes, instruments, and command/value editing exist.", "Compared with M8, the weakness is not the basic phrase concept; it is discoverability, command help, and cramped visibility.", "Use PLAY:PHR for focused writing and PLAY:SONG to hear the phrase against drums/bass."]
    },
    instrument: {
      purpose: "Instrument View edits the current sample/MIDI instrument parameters.",
      workflow: "Move through the parameter list, edit values, and open sample import from the sample field when assigning WAV material.",
      navigation: "R+Left returns Phrase; R+Down opens the instrument table; A on sample/import opens the sample browser.",
      fields: ["sample", "volume/pan/root", "filter", "drive/crush/downsample", "loop/start/end"],
      tips: ["This screen scrolls vertically through many parameters; it is not all visible at once.", "Native M8 synth engines are not present yet."]
    },
    sampler: {
      purpose: "The RG sample path is currently import/preview, not a full M8-style sample editor.",
      workflow: "Choose Listen to preview a WAV, Import to copy it into the project/sample pool, then assign it to the instrument.",
      navigation: "Opened from Instrument View. D-pad moves through files/actions; A activates; exit returns to Instrument.",
      fields: ["Listen", "Import", "file list", "selected sample"],
      tips: ["Deep crop/normalize/slice editing is a major missing workflow."]
    },
    table: {
      purpose: "Table View is RG Nano's strongest automation/movement tool.",
      workflow: "Enter command/value rows to create arps, pitch motion, retrigs, filter moves, and repeatable effects.",
      navigation: "Phrase table returns to Phrase; instrument table returns to Instrument. Select on command columns opens the command picker. Start plays phrase/current-channel context.",
      fields: ["row", "command", "value", "PLAY label"],
      tips: ["This is the closest current substitute for M8 modulation.", "PLAY:PHR means the table is being checked through phrase playback."]
    },
    groove: {
      purpose: "Groove View edits timing ticks for swing and rhythmic feel.",
      workflow: "Change tick values, then return to Phrase/Chain composition with the groove active.",
      navigation: "R+Up/route returns to Phrase in the audited flow.",
      fields: ["step", "tick value"],
      tips: ["Small groove edits are a high-impact way to make loops less rigid."]
    },
    mixer: {
      purpose: "Mixer View monitors all 8 channels and the master waveform.",
      workflow: "Use it while playback runs to see channel activity, per-channel scopes, and master movement.",
      navigation: "Route from Song to Mixer and back to Song. It is a monitor surface more than a full M8 mixer today.",
      fields: ["channels 00-07", "level boxes", "activity scopes", "master waveform", "PLAY label"],
      tips: ["All 8 channels fit in this view.", "EQ/limiter/send controls still need parity work."]
    },
    project: {
      purpose: "Project View holds song-level controls for save/load, tempo, render mode, key, scale, and note spelling.",
      workflow: "Set musical context here before writing, or return here to save/render after arranging.",
      navigation: "R+Up returns Song in the compact graph. Fields scroll vertically because the RG screen cannot show every setting at once.",
      fields: ["tempo", "key", "scale", "notes", "render", "save/load"],
      tips: ["New projects default to Key: -- for normal chromatic editing.", "Key/Scale guide future edits; they do not rewrite existing notes."]
    },
    render: {
      purpose: "RG Nano render turns the current project playback into WAV files.",
      workflow: "In Project View set Render to Stereo or Stems, press Start to play/render, then press Start again to stop and close the WAV.",
      navigation: "Use Project View for now. Stereo writes mixdown.wav in the active project folder; Stems writes channel0.wav, channel1.wav, and so on.",
      fields: ["Off", "Stereo", "Stems", "mixdown.wav", "channelN.wav"],
      tips: ["The simulator now verifies both native file outputs.", "M8-style row range, output naming, and render-to-sample are still missing."]
    },
    power: {
      purpose: "Power Menu is the global RG Nano app-exit overlay, not a tracker composition screen.",
      workflow: "Use Power when you need to leave the app or cancel back to the tracker.",
      navigation: "On real RG Nano hardware, Menu/Power opens the app exit/debug overlay. LGPT's universal helper is R+FN/Select, not Menu/Power.",
      fields: ["Exit", "Cancel"],
      tips: ["This screen exists to protect normal music input from accidental app exits."]
    },
    command: {
      purpose: "Command selection exists in command columns, but the user-facing help flow is not fully documented/tested.",
      workflow: "Use command fields in Phrase/Table, then open selector/help where available.",
      navigation: "Needs a confirmed universal or contextual shortcut.",
      fields: ["command", "value", "selector/help"],
      tips: ["This is a P0/P1 usability target because tracker commands are hard to memorize."]
    },
    modulation: {
      purpose: "No dedicated M8-style modulation screen exists on RG Nano yet.",
      workflow: "Use tables and instrument tables as the current movement workaround.",
      navigation: "Not a real RG screen.",
      fields: ["not present"],
      tips: ["Future design should be compact and table-aware."]
    },
    pool: {
      purpose: "No dedicated instrument pool exists on RG Nano yet.",
      workflow: "Manage instruments through the existing Instrument View until a compact pool/browser is built.",
      navigation: "Not a real RG screen.",
      fields: ["not present"],
      tips: ["Needed once sample-heavy projects become common."]
    },
    scale: {
      purpose: "No dedicated scale page exists yet; Project View provides Key, Scale, and Notes fields.",
      workflow: "Set Key/Scale in Project, then use scale-aware note editing in Phrase.",
      navigation: "Use Project View.",
      fields: ["Key", "Scale", "Notes"],
      tips: ["A dedicated compact Scale screen remains a good future target."]
    },
    eq: {
      purpose: "No dedicated EQ/Limiter screen exists on RG Nano yet.",
      workflow: "Use current instrument filters/effects and Mixer visuals until parity UI is built.",
      navigation: "Not a real RG screen.",
      fields: ["not present"],
      tips: ["Research existing filter/FX code before designing this."]
    },
    fx: {
      purpose: "FX exist through tracker commands/effects, but a dedicated M8-style FX settings screen is not audited.",
      workflow: "Use command columns and instrument/table automation for now.",
      navigation: "Needs code audit.",
      fields: ["commands", "values", "instrument/table automation"],
      tips: ["Do not promise a dedicated page yet."]
    }
  }
};

const rgRelatedCaptures = {
  boot: ["assets/rg-project-select.png", "assets/rg-new-project.png"],
  song: ["assets/rg-song-empty.png", "assets/rg-song-chain.png"],
  live: ["assets/rg-song-chain.png", "assets/rg-mixer-live.png"],
  chain: ["assets/rg-chain-empty.png", "assets/rg-chain-phrase.png"],
  phrase: ["assets/rg-phrase-empty.png", "assets/rg-phrase-note.png"],
  instrument: ["assets/rg-instrument.png", "assets/rg-sample-import.png"],
  sampler: ["assets/rg-sample-import.png"],
  table: ["assets/rg-table-phrase.png", "assets/rg-table-instrument.png"],
  mixer: ["assets/rg-mixer-live.png", "assets/rg-mixer-screen.png"],
  project: ["assets/rg-project.png"],
  render: ["assets/rg-project.png"],
  command: ["assets/rg-phrase-note.png", "assets/rg-table-phrase.png"],
  scale: ["assets/rg-project.png", "assets/rg-phrase-note.png"],
  power: ["assets/rg-power-menu.png"]
};

const screenExplainers = {
  song: {
    fl: "Closest FL Studio idea: Playlist. The Song screen decides when each track/channel starts playing a pattern.",
    concept: "Rows are time. Columns are tracks. A cell contains a chain number, not notes. Chain 00 means 'play whatever is inside chain 00' on this track at this row.",
    read: ["T128 or TEMPO 128 = project tempo in BPM", "00, 01, 02 down the left = song rows, like playlist positions", "1-8 or 00-07 across the grid = tracks/channels", "-- = empty cell, so that track does not start a chain there", "bottom RG boxes = live playback/channel activity feedback"],
    producer: "Use this when deciding structure: intro, main part, variation, breakdown, outro."
  },
  chain: {
    fl: "Closest FL Studio idea: a small Playlist clip stack for one channel. It says which patterns happen back-to-back before returning to Song.",
    concept: "A Song cell points to a Chain. A Chain points to one or more Phrases. It is a middle layer so one Song cell can trigger several phrase patterns in order.",
    read: ["Chain 00 = the chain currently being edited", "00-0F down the left = positions inside this chain", "PH or phrase value = which phrase pattern plays at that position", "TSP/transpose = pitch shift for that phrase", "-- = no phrase on that chain row", "1...... through 8...... = channel meters, not extra chains"],
    producer: "Use this to say: on this track, play phrase 00, then phrase 01, then maybe a fill. The right-side meters only tell you what channels are active while you edit."
  },
  phrase: {
    fl: "Closest FL Studio idea: Piano Roll plus Channel Rack step sequencer for one tracker track.",
    concept: "This is where the actual note events are. Each row is a step in the phrase. The fields on that row say what note plays, which instrument plays it, and whether any tracker command changes the sound.",
    read: ["00-0F down the left = phrase steps", "C-3, D#4, Bb2 = notes/pitches", "--- = rest/no note", "instrument number = which sound slot plays", "VOL/PIT/TBL/etc = command", "value after command = how strong or what setting that command uses"],
    producer: "Use this to write the melody, bassline, drum hits, chords split across tracks, and per-step effects."
  },
  instrument: {
    fl: "Closest FL Studio idea: Channel settings or plugin wrapper.",
    concept: "An instrument is the sound slot a phrase row triggers. M8 can pick synth engines or sampler engines. RG Nano currently focuses on sample/MIDI-style instruments.",
    read: ["type/engine = what makes the sound", "sample = audio file assigned to the instrument", "volume/pan/root = basic playback setup", "filter/drive/loop = sound-shaping controls", "table = optional command motion attached to the instrument"],
    producer: "Use this when the note is right but the sound itself needs changing."
  },
  sampler: {
    fl: "Closest FL Studio idea: Edison/Sampler/audio clip tools.",
    concept: "Sampler workflow loads or edits audio material so it can become an instrument.",
    read: ["Listen = preview a sample", "Import = copy/assign it into the project", "start/end/loop/slice = which part of the audio plays and how it repeats"],
    producer: "RG Nano can import and preview WAVs, but M8-style crop, normalize, slice, and selection-to-sample are still parity gaps."
  },
  table: {
    fl: "Closest FL Studio idea: automation pattern or per-step event editor.",
    concept: "A table is a tiny sequence of commands that can repeat quickly to add motion: arps, retrigs, pitch bends, filter moves, volume changes.",
    read: ["row = table step", "command = what changes", "value = command amount/target"],
    producer: "Use this when a simple note should move or animate without manually writing the same command over many phrase rows."
  },
  groove: {
    fl: "Closest FL Studio idea: swing/groove timing.",
    concept: "Groove changes how long steps last, making timing feel pushed, pulled, swung, or uneven.",
    read: ["step = groove slot", "tick value = how long that slot lasts"],
    producer: "Use this after a beat works but feels too rigid."
  },
  mixer: {
    fl: "Closest FL Studio idea: Mixer meters and channel levels.",
    concept: "Mixer/monitor screens show which tracks are active and how loud they are. M8 has deeper level/send/EQ/limiter tools than RG Nano today.",
    read: ["channel labels = tracks 1-8 or 00-07", "bars/boxes = level or activity", "waveform = master output movement"],
    producer: "Use this to catch dead tracks, too-loud channels, and overall movement while playback runs."
  },
  project: {
    fl: "Closest FL Studio idea: project settings plus export dialog.",
    concept: "Project holds settings that apply to the whole song: tempo, save/load, key/scale guidance, render/export mode.",
    read: ["tempo = BPM", "key/scale/notes = note-entry guidance/display", "render = bounce/export behavior", "save/load = project file actions"],
    producer: "Use this before writing for tempo/key setup, and after writing for saving or rendering."
  },
  render: {
    fl: "Closest FL Studio idea: Export WAV/MP3.",
    concept: "Render turns the tracker project into audio files.",
    read: ["source/range = what part to export", "stereo/stems = one mix or separate track files", "format/depth/name = output file settings"],
    producer: "This is a major RG Nano parity target because a portable studio needs a clean way to finish audio."
  },
  command: {
    fl: "Closest FL Studio idea: per-note automation/event commands.",
    concept: "Commands are short tracker codes on phrase/table rows. They change volume, pitch, table playback, delay, filter, retriggering, and other behavior.",
    read: ["command column = what action to perform", "value column = how much/which target", "help/selector = command lookup"],
    producer: "This needs better RG documentation because commands are powerful but hard to memorize."
  },
  modulation: {
    fl: "Closest FL Studio idea: LFO/envelope modulation inside a synth plugin.",
    concept: "M8 has dedicated modulation pages. RG Nano does not yet, so tables are the current workaround.",
    read: ["source = LFO/envelope/etc", "destination = parameter to move", "amount/speed = shape of movement"],
    producer: "Missing on RG Nano as a dedicated screen."
  },
  pool: {
    fl: "Closest FL Studio idea: channel/plugin/sample browser for instruments in the project.",
    concept: "Instrument Pool manages sound slots so a project does not become a mystery pile of numbers.",
    read: ["instrument number = slot", "name/type = what sound it is", "copy/manage = organize actions"],
    producer: "Missing on RG Nano as a dedicated screen."
  },
  scale: {
    fl: "Closest FL Studio idea: scale highlighting or note helpers in Piano Roll.",
    concept: "Scale guidance limits or labels note entry so melodies stay in a key unless you override.",
    read: ["root/key = tonal center", "scale/mode = allowed note family", "notes = spelling/display such as sharps or flats"],
    producer: "RG Nano has project fields for this, but not a dedicated scale screen."
  },
  eq: {
    fl: "Closest FL Studio idea: EQ and limiter on mixer inserts/master.",
    concept: "EQ shapes tone. Limiter controls peaks/loudness. M8 has dedicated pages; RG Nano does not yet.",
    read: ["low/mid/high = tone bands", "limit/gain = output control"],
    producer: "Missing on RG Nano as a dedicated workflow."
  },
  fx: {
    fl: "Closest FL Studio idea: send effects like delay/reverb plus plugin settings.",
    concept: "M8 has global effect settings and sends. RG Nano has tracker commands/effects, but a dedicated FX settings page is not proven.",
    read: ["time/feedback/mix = delay/reverb style parameters", "send = how much a sound goes into the effect"],
    producer: "Needs RG code and workflow audit."
  },
  live: {
    fl: "Closest FL Studio idea: performance mode / clip launching.",
    concept: "Live Mode cues chains while playback runs so you can test arrangements without writing them into Song rows first.",
    read: ["track = channel being cued", "queued chain = what will launch", "running state = what is currently playing"],
    producer: "M8 has this clearly. RG Nano live cueing still needs proof and polish."
  },
  boot: {
    fl: "Closest FL Studio idea: opening or creating a project.",
    concept: "This is the app start screen.",
    read: ["Load = open existing project", "New = create project", "Exit = leave app"],
    producer: "Use this before music work begins."
  },
  power: {
    fl: "Closest FL Studio idea: app/system exit confirmation.",
    concept: "This is not music data. It protects the app from accidental exits.",
    read: ["Exit = close app", "Cancel = return to tracker"],
    producer: "Use only when leaving the app."
  },
  system: {
    fl: "Closest FL Studio idea: audio/MIDI/general settings.",
    concept: "Device-level settings that affect the workstation, not the notes.",
    read: ["audio/display/input/MIDI/storage settings"],
    producer: "Usually set-and-forget."
  },
  theme: {
    fl: "Closest FL Studio idea: UI theme/color settings.",
    concept: "Visual preferences only.",
    read: ["background/text/highlight/cursor colors"],
    producer: "Low priority for RG Nano parity."
  },
  midi: {
    fl: "Closest FL Studio idea: MIDI settings and controller mapping.",
    concept: "Connects the tracker to external controllers, clocks, and instruments.",
    read: ["clock = sync timing", "channels = MIDI routing", "CC map = controller assignments"],
    producer: "RG Nano hardware support needs audit before promising this."
  }
};

const fieldComparisons = {
  song: [
    ["Tempo / T128", "Playback speed in beats per minute. T128 means 128 BPM.", "Visible on M8 Song/Project. Edit from Project tempo.", "Visible/editable on RG Project as TEMPO. From Song, go to Project and change TEMPO."],
    ["Song rows", "Time positions in the arrangement. Later rows happen later.", "Left side row numbers in Song View.", "Left side row numbers in RG Song. Scroll down for more of the 256 rows."],
    ["Track columns", "Parallel lanes. Values on the same row start at the same time, one per track.", "8 track columns across Song View.", "8 channel columns fit on RG Song as 00-07 / 1-8 style columns."],
    ["Chain number", "Instruction that says which chain this track should start playing.", "Cell value such as 00, 01, 1A, etc.", "Same concept. Select a cell, enter/edit chain number, then open Chain with R+Right."],
    ["--", "Empty cell. Nothing new starts on that track at that row.", "Shown as -- in empty Song cells.", "Shown as -- in empty RG Song cells."],
    ["Playback feedback", "Shows what is currently active while the song plays.", "M8 has position/status indicators and meters.", "RG bottom boxes/waveform show channel activity so you can keep editing while listening."]
  ],
  chain: [
    ["Chain 00 / CHAIN 00", "The chain number currently being edited.", "Header shows the current chain.", "Header shows the current chain. This is the chain selected from the Song cell."],
    ["00-0F down the left", "The 16 positions inside this chain. Position 00 plays before 01.", "Left-side chain row numbers.", "Left-side chain row numbers. Move up/down to choose a row."],
    ["PH / phrase value", "Which phrase pattern plays at this chain position.", "PH column on M8 Chain.", "Middle data column on RG Chain. Press A/edit, then R+Right opens that phrase."],
    ["TSP / transpose", "Pitch shift for that phrase when it plays from this chain.", "TSP column on M8 Chain.", "Right data column on RG Chain."],
    ["--", "No phrase at this chain position.", "Shown when a chain row is empty.", "Shown when a chain row is empty."],
    ["1...... through 8......", "Live channel meters, not chain data. Number = channel. Dots = quiet/empty meter.", "M8 shows channel/status information on the right side.", "RG Chain draws 8 channel meters on the right. During playback dots can become #; muted channels show dashes."],
    ["> or play marker", "Shows the chain row currently playing.", "M8 marks the current play row.", "RG code draws a play marker beside the active row while playback runs."]
  ],
  phrase: [
    ["Step row", "A small time step inside the phrase pattern.", "Rows 00-0F in Phrase View.", "Rows 00-0F in RG Phrase. Move up/down through steps."],
    ["Note", "Pitch to play, or a rest if blank.", "Note field like C-3, D#4, ---.", "Same idea. A+D-pad edits; L+D-pad can do chromatic override when scale guidance is active."],
    ["Instrument", "Which sound slot plays this note.", "Instrument number field beside the note.", "Instrument number field. R+Right opens Instrument to edit that sound."],
    ["Volume / velocity", "How loud this step plays.", "Dedicated field or command depending M8 context.", "Use available volume field/command. Command help still needs clearer RG tutorial."],
    ["Command", "A short instruction like volume, pitch, table, delay, retrigger.", "FX command columns on M8 Phrase.", "Command/value pairs on RG Phrase. Core editing exists, but discoverability is weaker than M8."],
    ["Mini waveform/activity", "Playback movement while writing.", "M8 has richer status/scope information.", "RG mini waveform/activity strip gives feedback without leaving Phrase."]
  ],
  instrument: [
    ["Engine/type", "What creates the sound.", "M8 can choose sampler and native synth engines.", "RG currently focuses on sample/MIDI-style instruments. Native M8-like synth engines are missing."],
    ["Sample", "Audio file assigned to this instrument.", "Sampler/sample fields and editor routes.", "Sample field opens/imports uploaded WAVs from RG sample import path."],
    ["Volume/pan/root", "Basic playback setup for the sound.", "Instrument parameters.", "RG Instrument parameters, vertically scrolled on the small screen."],
    ["Filter/drive/loop", "Tone and playback shaping.", "M8 instrument engine/editor fields.", "Some sample/instrument shaping exists; parity needs field-by-field audit."],
    ["Table/modulation", "Extra movement attached to the sound.", "M8 has tables and dedicated modulation.", "RG has tables, but no dedicated M8-style modulation page yet."]
  ],
  sampler: [
    ["Preview/listen", "Hear a sample before using it.", "M8 sampler can audition material.", "RG Import Sample dialog has Listen."],
    ["Import/load", "Bring audio into the project.", "M8 loads/records samples.", "RG Import copies/assigns uploaded WAVs. Live recording is out of scope for RG Nano right now."],
    ["Start/end/loop", "Choose which part of the sample plays.", "M8 Sample Editor.", "RG has some instrument/sample parameters, but deep editor parity is incomplete."],
    ["Crop/normalize/slice", "Permanent editing and chopping tools.", "M8 Sample Editor supports deep sample work.", "Missing/major RG parity target."]
  ],
  table: [
    ["Table row", "Step inside a command mini-sequence.", "Rows in M8 Table.", "Rows in RG Table / Instrument Table."],
    ["Command", "What parameter or action changes.", "Command field.", "Command field. Selector/help path needs better docs."],
    ["Value", "Amount or target for the command.", "Value field.", "Value field."],
    ["Attach/trigger", "How the table gets used by a note/instrument.", "Phrase or instrument table routes.", "RG Phrase Table and Instrument Table exist; use them as current modulation workaround."]
  ],
  groove: [
    ["Groove step", "Position in the timing pattern.", "M8 Groove row.", "RG Groove row."],
    ["Ticks", "How long that timing step lasts.", "Tick value.", "Tick value. Higher/lower values make timing feel pushed, pulled, or swung."]
  ],
  mixer: [
    ["Channel label", "Which track the meter/control belongs to.", "M8 track/input/send/output labels.", "RG channels 00-07."],
    ["Level/meter", "How active or loud the channel is.", "M8 mixer bars/values.", "RG level boxes/scopes show activity. Full level-edit parity still needs audit."],
    ["Master waveform", "Shape/movement of final audio output.", "M8 scope/monitor pages.", "RG Mixer and Phrase mini waveform show output movement."],
    ["EQ/limiter route", "Tone/loudness control after levels.", "Dedicated M8 EQ/Limiter pages.", "Not dedicated on RG yet."]
  ],
  project: [
    ["Tempo", "Song speed in BPM.", "M8 Project/Song tempo display such as T128.", "RG Project TEMPO. From Song, go to Project, select TEMPO, edit value."],
    ["Key/Scale/Notes", "Optional note guidance and spelling.", "M8 Scale View plus project context.", "RG Project Key, Scale, Notes fields. New projects can stay Key -- for normal chromatic editing."],
    ["Save/load", "Project file management.", "M8 Project actions.", "RG Project/selector actions."],
    ["Render mode", "How to bounce/export audio.", "M8 Render View from Project.", "RG Project render fields exist, but finished output flow still needs proof."]
  ],
  render: [
    ["Source/range", "Which part of the song to export.", "M8 Render View fields.", "RG needs a clearer render route and file assertion."],
    ["Stereo/stems", "One mixed file or separate track files.", "M8 Render View supports render options.", "RG has render mode fields like Stereo/Stems but needs end-to-end UX."],
    ["Completion/output path", "Where the audio file ends up.", "M8 render workflow.", "RG needs visible completion feedback and documented output location."]
  ],
  command: [
    ["Command code", "Short text instruction for a tracker effect.", "M8 command help explains codes.", "RG command columns exist; help/selector needs a clearer universal path."],
    ["Value", "Parameter for that command.", "Value column beside command.", "Value column beside command."],
    ["Help/selector", "Lookup so users do not memorize everything.", "M8 has Effect Command Help.", "RG needs proven shortcut/tutorial for this."]
  ],
  scale: [
    ["Root/key", "Main note center.", "M8 Scale View.", "RG Project Key field."],
    ["Scale/mode", "Allowed note set.", "M8 Scale View.", "RG Project Scale field."],
    ["Break out of key", "Enter a note outside the guide.", "M8 supports normal tracker note entry/context.", "RG L+D-pad chromatic override while editing notes."]
  ],
  eq: [
    ["EQ bands", "Tone shaping ranges.", "M8 EQ Editor.", "No dedicated RG screen yet."],
    ["Limiter/scope", "Peak/loudness safety and output view.", "M8 Limiter & Mix Scope.", "RG has visual scopes, but no full limiter workflow yet."]
  ],
  fx: [
    ["Effect type/settings", "Delay/reverb/mod effect character.", "M8 Effect Settings.", "RG effect path through commands/tables needs audit."],
    ["Send/mix", "How much signal goes to the effect.", "M8 mixer/instrument send workflow.", "No polished RG parity workflow yet."]
  ],
  live: [
    ["Queued chain", "Chain prepared to launch while playing.", "M8 Live Mode.", "RG Live Mode is marked partial/needs proof."],
    ["Track launch state", "Which tracks are running or waiting.", "M8 Live Mode.", "RG needs dedicated capture and shortcut confirmation."]
  ]
};

const programmaticModels = {
  song: "Programmatic model: an array of song rows. Each row has 8 slots. Each slot stores a chain id or empty.",
  chain: "Programmatic model: an ordered list of phrase ids, with optional transpose values.",
  phrase: "Programmatic model: a 16-step event list. Each step can contain note, instrument id, and command/value pairs.",
  instrument: "Programmatic model: a sound definition object referenced by phrase events.",
  sampler: "Programmatic model: audio files plus playback ranges and loop/slice metadata.",
  table: "Programmatic model: a tiny command sequence that can be triggered by a phrase or instrument.",
  groove: "Programmatic model: a timing table that changes step durations.",
  mixer: "Programmatic model: per-channel activity/level state plus master output state.",
  project: "Programmatic model: global project settings such as tempo, key, save state, and render mode.",
  render: "Programmatic model: export settings that turn project data into audio files.",
  command: "Programmatic model: compact opcodes plus values attached to phrase/table steps.",
  scale: "Programmatic model: a note-filter/display rule used during note entry.",
  eq: "Programmatic model: tone and dynamics processors on tracks or master output.",
  fx: "Programmatic model: effect processors and send/mix amounts.",
  live: "Programmatic model: queued chain ids per track while playback is running."
};

const flowStepGuides = {
  "Build first pattern": [
    {
      m8: "Your first pattern starts by choosing one place in the Song grid. Pick row 00 if you are starting from scratch, then pick one of the 8 track columns. Enter chain 00 in that cell. T128 means the project tempo is 128 BPM.",
      rg: "On RG Nano, Song View works the same idea in a tighter space: row = time, column = channel, cell = chain number. All 8 channels fit. The bottom volume/note boxes are playback feedback so you can see which channels are active while listening."
    },
    {
      m8: "A chain is a playlist for that one track. If chain 00 contains phrase 00 on its first row, then the song cell '00' means: play chain 00, and chain 00 starts by playing phrase 00.",
      rg: "RG Chain is the same layer. It does not contain notes yet. It says which phrase numbers this track should play, in order. Press A to enter/edit a phrase number, then R+Right opens that phrase."
    },
    {
      m8: "A phrase is the actual pattern. Rows are steps in time. Columns are fields for the event on that step: note, instrument, volume/velocity, and command/value effects.",
      rg: "RG Phrase has the same core job: notes, instruments, and command/value pairs. It is not missing the basic phrase idea, but the tiny screen makes fields tighter and command discovery needs more help."
    },
    {
      m8: "Playback loops the current context so you hear whether the phrase, chain, or song section works. The screen position tells you what is currently playing.",
      rg: "Use Start for focused playback and R+Start for song context from Chain/Phrase-style screens. The RG PLAY label tells you what you are hearing: PLAY:SONG for arrangement context, PLAY:CHAIN for chain scope, PLAY:PHR for phrase scope, and AUDITION for previews."
    }
  ],
  "Build instruments": [
    {
      m8: "M8 instruments can be sampler, synth, MIDI, or external-input engines. The Instrument screen is where the sound source and core parameters are chosen.",
      rg: "RG Nano currently has sample/MIDI-style instrument editing. Open Instrument from Phrase, move through the parameter list, and use the sample field to reach import."
    },
    {
      m8: "The M8 Sampler/Sample Editor can load, record, slice, loop, crop, normalize, and process samples.",
      rg: "RG Nano supports uploaded WAV import and preview. Use Listen before Import, then assign the imported WAV to the current instrument. Deep sample editing is still missing."
    },
    {
      m8: "M8 modulation uses dedicated instrument modulation pages for LFO/envelope/tracking movement.",
      rg: "RG Nano does not have that page yet. Use Phrase Table or Instrument Table commands for repeating motion until a compact modulation view exists."
    }
  ],
  "Arrange full song": [
    {
      m8: "Arranging starts after you have a loop. Copy the phrase or chain, change a few notes, and now you have a variation: fill, answer, breakdown, or ending.",
      rg: "RG uses the same concept. Make a second chain or phrase for the variation, then place that new chain number later in Song View."
    },
    {
      m8: "A song is not a separate audio file at this point. It is the Song grid telling each track which chain to play at each row: intro rows, main rows, breakdown rows, outro rows.",
      rg: "On RG Nano, arrangement means scrolling down Song rows and placing different chain numbers. Columns still mean tracks; rows still mean time."
    },
    {
      m8: "Groove changes timing feel. Tables repeat command motion. These polish a loop or arrangement after the notes exist.",
      rg: "RG has Groove and Tables too. For loop writing, these are important because they add swing, pitch movement, retrigs, and effect motion without requiring new synth engines."
    }
  ],
  "Mix and monitor": [
    {
      m8: "M8 Mixer shows track/input/send/output levels and leads to EQ, limiter/scope, and FX settings.",
      rg: "RG Mixer currently focuses on monitoring: all 8 channels, scopes, and master waveform. Full level/EQ/limiter parity is still a build target."
    },
    {
      m8: "M8 has dedicated EQ and limiter/scope pages for tone shaping and output control.",
      rg: "RG Nano does not have a dedicated EQ/Limiter page yet. Use instrument filters/effects for now; design work should start from existing filter and mixer code."
    },
    {
      m8: "M8 global FX settings define effect character, while tracks/instruments send into those effects.",
      rg: "RG FX capability needs audit. Commands and tables exist, but a clear M8-style FX settings workflow is not proven."
    }
  ],
  "Render a track": [
    {
      m8: "Project View is the hub for song-level work and leads to Render View.",
      rg: "RG Project View holds tempo, save/load, key, scale, note spelling, and render mode. Fields scroll vertically because the RG screen is tiny."
    },
    {
      m8: "Render View exports a WAV, including mixed output or stems with row range, repeats, effects, and name options.",
      rg: "RG Project Render exports Stereo as mixdown.wav and Stems as channelN.wav files. The current sim test proves both outputs after Start/Stop."
    },
    {
      m8: "Selection-to-sample turns selected material into a new sample for reuse, freeing tracks and enabling resampling workflows.",
      rg: "RG Nano does not have selection-to-sample yet. This is especially valuable because we are avoiding live sampling for now."
    }
  ]
};

const gaps = [
  ["P0", "Command selector tutorial", "Exists in RG command columns, but needs a clear flow and test."],
  ["P1", "Render/export UX", "Stereo/stems work; still needs range, naming, visible destination, and render-to-sample."],
  ["P1", "Sample editor", "RG imports samples but lacks crop, normalize, slice, and resample UI."],
  ["P1", "Instrument pool", "Not in RG Nano yet."],
  ["P2", "Native synth/modulation", "M8 engines and modulation are not in RG Nano yet."],
  ["P2", "EQ/limiter", "Not in RG Nano yet; only scope visuals exist."]
];

let activeFlow = 0;
let activeStep = 0;
let activeFlowDevice = "m8";
let activeMap = document.body.dataset.mapPage || "m8";
let activeScreen = activeMap === "rg" ? "boot" : "song";

function renderMock(el, screen, flavor) {
  el.innerHTML = `
    <div class="mock-top"><strong>${screen.name}</strong><em>${flavor}</em></div>
    <div class="mock-wave"></div>
    <div class="mock-fields">${screen.columns.slice(0, 4).map(x => `<span>${x}</span>`).join("")}</div>
    <div class="mock-rows">${screen.rows.map((r, i) => `<div class="mock-row ${i === 0 ? "selected" : ""}">${r}</div>`).join("")}</div>
  `;
}

function screenIdFor(screen) {
  return Object.keys(screens).find(id => screens[id] === screen) || "song";
}

function renderRg(el, screen) {
  const img = document.getElementById("rgCapture");
  const fallback = document.getElementById("rgFallback");
  if (screen.capture) {
    img.src = screen.capture;
    img.hidden = false;
    fallback.hidden = true;
  } else if (screen.status === "missing") {
    img.hidden = true;
    fallback.hidden = false;
    fallback.innerHTML = `<div class="missing-screen"><span>Not in RG Nano yet</span><strong>${screen.name}</strong><p>${screen.summary}</p></div>`;
  } else if (screen.status === "partial") {
    img.hidden = true;
    fallback.hidden = false;
    fallback.innerHTML = `<div class="missing-screen partial-note"><span>Partial / needs audit</span><strong>${screen.name}</strong><p>${screen.summary}</p></div>`;
  } else {
    img.hidden = true;
    fallback.hidden = false;
    renderMock(fallback, screen, "RG");
  }
}

function renderFlows() {
  document.getElementById("flowButtons").innerHTML = flows.map((f, i) => `
    <button class="${i === activeFlow ? "active" : ""}" data-flow="${i}"><strong>${f.title}</strong><span>${f.summary}</span></button>
  `).join("");
}

function renderStage() {
  const flow = flows[activeFlow];
  const [title, desc, m8Id, rgId, m8Ctrl, rgCtrl] = flow.steps[activeStep];
  const m8 = screens[m8Id];
  const rg = screens[rgId];
  document.getElementById("flowTitle").textContent = flow.title;
  document.getElementById("flowSummary").textContent = flow.summary;
  document.getElementById("stepTitle").textContent = title;
  document.getElementById("stepCounter").textContent = `${activeStep + 1} / ${flow.steps.length}`;
  document.getElementById("m8Title").textContent = m8.name;
  document.getElementById("rgTitle").textContent = rg.name;
  document.getElementById("m8Controls").textContent = `M8: ${m8Ctrl}`;
  document.getElementById("rgControls").textContent = `RG Nano: ${rgCtrl}`;
  document.getElementById("stepDescription").textContent = desc;
  document.getElementById("m8Display").innerHTML = renderM8Reference(m8Id);
  renderRg(document.getElementById("rgFallback"), rg);
  renderScreenExplainer(m8Id, rgId);
  document.getElementById("stepList").innerHTML = flow.steps.map((s, i) => `
    <li class="${i === activeStep ? "active" : ""}"><button data-step="${i}"><strong>${s[0]}</strong><span>${screens[s[2]].name} -> ${screens[s[3]].name}</span></button></li>
  `).join("");
  renderFlowManual();
  renderFlows();
}

function renderScreenExplainer(m8Id, rgId) {
  const panel = document.getElementById("screenExplainer");
  if (!panel) return;
  const primary = screenExplainers[rgId] || screenExplainers[m8Id];
  const m8 = screenExplainers[m8Id];
  const rg = screenExplainers[rgId];
  const parity = m8Id === rgId
    ? "Both devices are showing the same tracker layer here."
    : `M8 is showing ${screens[m8Id].name}; RG Nano is showing ${screens[rgId].name}. This means RG uses a different or smaller screen for this part.`;
  panel.innerHTML = `
    <p class="eyebrow">What am I looking at?</p>
    <div class="explainer-grid">
      <section>
        <h3>${screens[rgId].name}</h3>
        <p>${primary.concept}</p>
        <p>${programmaticModels[rgId] || programmaticModels[m8Id] || ""}</p>
      </section>
      <section>
        <h4>Read the screen</h4>
        <ul>${primary.read.map(item => `<li>${item}</li>`).join("")}</ul>
      </section>
      <section>
        <h4>How this maps</h4>
        <p>${primary.fl}</p>
        <p>${primary.producer}</p>
        <p>${parity}</p>
        ${m8Id !== rgId && m8 ? `<p>M8 side: ${m8.concept}</p>` : ""}
        ${m8Id !== rgId && rg ? `<p>RG side: ${rg.concept}</p>` : ""}
      </section>
    </div>
    ${renderFieldComparison(rgId, m8Id)}
  `;
}

function renderFieldComparison(screenId, fallbackScreenId = screenId) {
  const rows = fieldComparisons[screenId] || fieldComparisons[fallbackScreenId] || [];
  if (!rows.length) return "";
  return `
    <section class="field-compare">
      <h4>Screen legend</h4>
      <div>
        ${rows.map(([field, meaning, m8, rg]) => `
          <article>
            <strong>${field}</strong>
            <p>${meaning}</p>
            <dl>
              <dt>M8</dt><dd>${m8}</dd>
              <dt>RG Nano</dt><dd>${rg}</dd>
            </dl>
          </article>
        `).join("")}
      </div>
    </section>
  `;
}

function renderFlowManual() {
  const panel = document.getElementById("flowManual");
  if (!panel) return;
  const flow = flows[activeFlow];
  const guide = flowStepGuides[flow.title]?.[activeStep];
  const [title, , m8Id, rgId, m8Ctrl, rgCtrl] = flow.steps[activeStep];
  const device = activeFlowDevice;
  const screenId = device === "m8" ? m8Id : rgId;
  const manual = screenManuals[device]?.[screenId] || {};
  document.querySelectorAll("[data-flow-device]").forEach(button => {
    button.classList.toggle("active", button.dataset.flowDevice === activeFlowDevice);
  });
  panel.innerHTML = `
    <p class="eyebrow">${device === "m8" ? "M8 step guide" : "RG Nano step guide"}</p>
    <h3>${title}</h3>
    <p>${guide?.[device] || manual.workflow || "No device-specific guide written yet."}</p>
    <dl>
      <dt>Screen</dt><dd>${screens[screenId]?.name || screenId}</dd>
      <dt>Controls</dt><dd>${device === "m8" ? m8Ctrl : rgCtrl}</dd>
      <dt>Fields</dt><dd>${manual.fields?.join("; ") || screens[screenId]?.columns?.join("; ") || "No fields listed yet."}</dd>
      <dt>What to watch</dt><dd>${manual.purpose || screens[screenId]?.summary || "No notes yet."}</dd>
      <dt>Navigation/window</dt><dd>${manual.navigation || "Follow the mapped route for this device."}</dd>
    </dl>
  `;
}

function renderManualBlock(device, screenId) {
  const manual = screenManuals[device]?.[screenId];
  if (!manual) return "";
  return `
    <section class="manual-block">
      <p class="eyebrow">${device === "m8" ? "M8 manual-style notes" : "RG Nano manual-style notes"}</p>
      <h4>${manual.purpose}</h4>
      <p>${manual.workflow}</p>
      <dl>
        <dt>Navigation/window</dt><dd>${manual.navigation}</dd>
        <dt>Fields</dt><dd>${manual.fields.join("; ")}</dd>
        <dt>Producer tips</dt><dd>${manual.tips.join("; ")}</dd>
      </dl>
      ${renderFieldComparison(screenId)}
    </section>
  `;
}

function renderRelatedCaptures(screenId) {
  const captures = rgRelatedCaptures[screenId] || [];
  if (!captures.length) return "";
  return `
    <section class="capture-strip">
      <p class="eyebrow">RG Nano related captures</p>
      <div>${captures.map(src => `<img src="${src}" alt="">`).join("")}</div>
    </section>
  `;
}

function mini(screenId, device) {
  const screen = screens[screenId];
  if (device === "m8" && m8ManualCrops[screenId]) {
    return renderM8Reference(screenId);
  }
  if (device === "rg" && screen.capture) return `<img src="${screen.capture}" alt="">`;
  if (device === "rg" && screen.status === "missing") return `<div class="mini-missing">Not in RG</div>`;
  if (device === "rg" && screen.status === "partial") return `<div class="mini-missing partial">Needs audit</div>`;
  return renderM8Screen(screenId);
}

function renderM8Reference(screenId) {
  const screen = screens[screenId];
  const crop = m8ManualCrops[screenId];
  if (!crop) return renderM8Screen(screenId);
  return `<div class="m8-reference-frame">
    <img class="m8-manual-crop" src="${crop}?v=${ASSET_VERSION}" alt="${screen.name} dark manual render" onerror="this.parentElement.classList.add('missing')">
    <div class="m8-reference-fallback">${renderM8Screen(screenId)}</div>
  </div>`;
}

function renderM8Screen(screenId) {
  const screen = screens[screenId];
  const thumb = m8Thumbs[screenId] || { kind: "grid", meta: "view", rows: screen.rows };
  const ref = m8ManualRefs[screenId];
  const meta = ref ? `${thumb.meta} · p${ref.page}${ref.endPage ? `-${ref.endPage}` : ""}` : thumb.meta;
  let body = "";
  if (thumb.kind === "bars") {
    body = `<div class="m8-bars">${thumb.bars.map(height => `<span style="height:${height}%"></span>`).join("")}</div>`;
  } else if (thumb.kind === "params") {
    body = `<div class="m8-params">${thumb.params.map(param => `<span>${param}</span>`).join("")}</div>`;
  } else if (thumb.kind === "wave") {
    body = `<div class="m8-wave"></div><div class="m8-grid-lines">${thumb.rows.map((row, i) => `<div class="m8-line ${i === 0 ? "sel" : ""}">${row}</div>`).join("")}</div>`;
  } else {
    body = `<div class="m8-grid-lines">${thumb.rows.map((row, i) => `<div class="m8-line ${i === 0 ? "sel" : ""}">${row}</div>`).join("")}</div>`;
  }
  return `<div class="m8-ui" data-kind="${thumb.kind}">
    <div class="m8-monitor" aria-hidden="true"></div>
    <div class="m8-ui-body">
      <main>
        <header><strong>${screen.name.toUpperCase()}</strong><span>${meta}</span></header>
        ${body}
      </main>
      <aside>
        <span>T=128</span>
        <b>1-A-3</b>
        <i></i>
        <em>SCPIT</em>
      </aside>
    </div>
  </div>`;
}

function renderFigmaBoard(graph, board, device) {
  const linked = new Set();
  board.edges.forEach(([from, to]) => {
    if (from === activeScreen) linked.add(to);
    if (to === activeScreen) linked.add(from);
  });
  const paths = board.edges.map(([from, to]) => {
    const a = board.nodes[from], b = board.nodes[to];
    const ax = a.x + a.w;
    const ay = a.y + a.h / 2;
    const bx = b.x;
    const by = b.y + b.h / 2;
    const bend = Math.max(70, Math.abs(bx - ax) * 0.42);
    const active = from === activeScreen || to === activeScreen;
    return `<path class="${active ? "active" : "dim"}" data-edge-from="${from}" data-edge-to="${to}" d="M ${ax} ${ay} C ${ax + bend} ${ay}, ${bx - bend} ${by}, ${bx} ${by}"></path>`;
  }).join("");

  graph.classList.add("figma-canvas");
  graph.style.minHeight = `${board.height + 24}px`;
  graph.innerHTML = `
    <div class="figma-board" style="width:${board.width}px;height:${board.height}px;">
      ${board.groups.map(group => `<section class="figma-group ${group.tone}" style="left:${group.x}px;top:${group.y}px;width:${group.w}px;height:${group.h}px;"><span>${group.label}</span></section>`).join("")}
      <svg class="figma-edge-layer" viewBox="0 0 ${board.width} ${board.height}">
        <defs><marker id="figmaArrow" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M 0 0 L 10 5 L 0 10 z"></path></marker></defs>
        ${paths}
      </svg>
      ${Object.entries(board.nodes).map(([id, node]) => {
        const s = screens[id];
        const status = device === "m8" ? "reference" : s.status;
        const ref = device === "m8" ? manualPageText(m8ManualRefs[id]) : "";
        return `<article class="figma-node ${node.size || ""} ${status} ${id === activeScreen ? "active" : ""} ${linked.has(id) ? "linked" : ""}" style="left:${node.x}px;top:${node.y}px;width:${node.w}px;height:${node.h}px;" data-graph-screen="${id}">
          <div class="figma-screen">${mini(id, device)}</div>
          <div>
            <strong>${s.name}</strong>
            <small>${ref ? `${node.note} · ${ref}` : node.note}</small>
          </div>
        </article>`;
      }).join("")}
    </div>
  `;
  const activeNode = board.nodes[activeScreen];
  if (activeNode) {
    requestAnimationFrame(() => {
      const left = Math.max(0, activeNode.x - Math.max(80, graph.clientWidth * 0.35));
      graph.scrollTo({ left, behavior: "smooth" });
    });
  }
  renderDetail();
}

function renderGraph() {
  const graph = document.getElementById("screenGraph");
  if (!graph) return;
  graph.classList.remove("figma-canvas");
  graph.style.minHeight = "";
  if (activeMap === "m8") {
    renderFigmaBoard(graph, m8Board, "m8");
    return;
  }
  if (activeMap === "rg" && document.body.dataset.mapPage === "rg") {
    renderFigmaBoard(graph, rgBoard, "rg");
    return;
  }
  const map = graphMaps[activeMap];
  const nodes = new Map(map.nodes.map(n => [n[0], n]));
  const edgeSvg = map.edges.map(([from, to]) => {
    const a = nodes.get(from), b = nodes.get(to);
    return `<path d="M ${a[1] + 5} ${a[2] + 4} L ${b[1] + 5} ${b[2] + 4}"></path>`;
  }).join("");
  graph.innerHTML = `
    <svg class="edge-layer" viewBox="0 0 100 100" preserveAspectRatio="none">
      <defs><marker id="arrow" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="3" markerHeight="3" orient="auto-start-reverse"><path d="M 0 0 L 10 5 L 0 10 z" fill="rgba(30,231,211,0.6)"></path></marker></defs>
      ${edgeSvg}
    </svg>
    ${map.nodes.map(([id, x, y]) => {
      const s = screens[id];
      const status = activeMap === "m8" ? "reference" : s.status;
      const ref = activeMap === "m8" ? manualPageText(m8ManualRefs[id]) : "";
      return `<article class="graph-node ${id === activeScreen ? "active" : ""} ${status}" style="left:${x}%;top:${y}%;" data-graph-screen="${id}">
        <div class="mini-screen">${mini(id, activeMap)}</div>
        <strong>${s.name}</strong>
        <small>${status === "reference" ? ref || "M8 reference" : s.status}</small>
      </article>`;
    }).join("")}
  `;
  renderDetail();
}

function renderDetail() {
  const detail = document.getElementById("screenDetail");
  if (!detail) return;
  const s = screens[activeScreen] || screens.song;
  const map = graphMaps[activeMap];
  const status = activeMap === "m8" ? "reference" : s.status;
  const m8RefLink = manualLink(activeScreen);
  const incoming = map.edges.filter(e => e[1] === activeScreen).map(e => screens[e[0]]?.name).filter(Boolean);
  const outgoing = map.edges.filter(e => e[0] === activeScreen).map(e => screens[e[1]]?.name).filter(Boolean);
  let preview = "";
  if (activeMap === "m8") {
    const crop = m8ManualCrops[activeScreen];
    preview = crop
      ? `<div class="detail-preview manual-crop-preview">${renderM8Reference(activeScreen)}</div>`
      : `<div class="detail-preview">${renderM8Screen(activeScreen)}</div>`;
  } else if (s.capture) {
    preview = `<div class="detail-preview"><img src="${s.capture}" alt=""></div>`;
  }
  detail.innerHTML = `
    <p class="eyebrow">${map.title}</p>
    <h3>${s.name}</h3>
    <span class="status ${status}">${status === "reference" ? "M8 reference" : s.status}</span>
    <p>${s.summary}</p>
    ${preview}
    ${renderManualBlock(activeMap === "m8" ? "m8" : "rg", activeScreen)}
    ${activeMap === "rg" ? renderRelatedCaptures(activeScreen) : ""}
    <dl>
      <dt>Reference</dt><dd>${s.ref}</dd>
      ${activeMap === "m8" && m8RefLink ? `<dt>M8 manual source</dt><dd>${m8RefLink}</dd>` : ""}
      <dt>Exists on this device</dt><dd>${activeMap === "m8" ? "Yes. This is part of the M8 reference workflow." : (s.status === "missing" ? "No. This RG Nano screen/workflow does not exist yet." : "Yes, with the status shown above.")}</dd>
      <dt>Numbers/columns</dt><dd>${s.columns.join("; ")}</dd>
      <dt>Controls</dt><dd>${s.actions.join("; ")}</dd>
      <dt>Comes from</dt><dd>${incoming.length ? incoming.join("; ") : "Root / entry screen"}</dd>
      <dt>Goes to</dt><dd>${outgoing.length ? outgoing.join("; ") : "No mapped outgoing route"}</dd>
    </dl>
  `;
}

function renderGaps() {
  const gapList = document.getElementById("gapList");
  if (!gapList) return;
  gapList.innerHTML = gaps.map(g => `<article><span>${g[0]}</span><h3>${g[1]}</h3><p>${g[2]}</p></article>`).join("");
}

document.body.addEventListener("click", (e) => {
  const flow = e.target.closest("[data-flow]");
  if (flow) { activeFlow = Number(flow.dataset.flow); activeStep = 0; renderStage(); }
  const step = e.target.closest("[data-step]");
  if (step) { activeStep = Number(step.dataset.step); renderStage(); }
  const flowDevice = e.target.closest("[data-flow-device]");
  if (flowDevice) { activeFlowDevice = flowDevice.dataset.flowDevice; renderStage(); }
  const device = e.target.closest("[data-device]");
  if (device) {
    activeMap = device.dataset.device;
    activeScreen = activeMap === "rg" ? "song" : "song";
    document.querySelectorAll("[data-device]").forEach(b => b.classList.toggle("active", b.dataset.device === activeMap));
    renderGraph();
  }
  const graphNode = e.target.closest("[data-graph-screen]");
  if (graphNode) { activeScreen = graphNode.dataset.graphScreen; renderGraph(); }
});

const prevStep = document.getElementById("prevStep");
const nextStep = document.getElementById("nextStep");
if (prevStep) prevStep.addEventListener("click", () => { activeStep = Math.max(0, activeStep - 1); renderStage(); });
if (nextStep) nextStep.addEventListener("click", () => { activeStep = Math.min(flows[activeFlow].steps.length - 1, activeStep + 1); renderStage(); });

if (document.getElementById("flowButtons")) {
  renderFlows();
  renderStage();
}
renderGraph();
renderGaps();
