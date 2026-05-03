# Tracker Producer Workflow Notes

## Writing A Melody Over Drums

A tracker song is a grid of time rows and channels. In this fork there are 8 channels. Each cell in the Song screen points to a chain. When multiple chains are placed on the same Song row, they play at the same time, one per channel.

A practical beat-plus-melody setup looks like this:

| Song row | Channel 0 | Channel 1 | Channel 2 |
| --- | --- | --- | --- |
| 00 | Drum chain 00 | Bass chain 01 | Melody chain 02 |

The drum chain is not the whole song. It is one channel's ordered list of phrases. The melody chain is another channel's ordered list of phrases. Because they are placed on the same Song row, playback runs them together.

When writing the melody, you do not have to hear it alone. You can enter the melody's phrase from the Song/Chain context and start playback from the Song row or current chain/phrase depending on what you need:

| Need | Tracker approach |
| --- | --- |
| Hear the full arrangement | Start from Song view, or use `R + Start` from Phrase/Chain-style editing screens to get song context. The screen should show `PLAY:SONG`. |
| Focus on a loop section | Keep drum, bass, and melody chains on the same Song row and loop/restart that row. |
| Edit only the melody notes | Move into the melody chain/phrase on its channel. Normal `Start` on Phrase focuses the current phrase/channel and should show `PLAY:PHR`. |
| Audition one note | Use note/instrument preview from Phrase view. |
| Build variation | Make another melody phrase or chain, then place it on a later Song row while the drum chain repeats or changes. |

The mental model is close to FL Studio patterns in a playlist:

- Song row = playlist time position.
- Channel = one lane/track.
- Chain = a short playlist of phrases for that channel.
- Phrase = the actual note/command pattern.
- Table = fast per-step automation/movement attached from a phrase or instrument.

So a drum loop and melody loop are not competing alternatives. They are parallel channel content. You compose them together by arranging their chains on the same Song rows, then editing the phrase you care about while playback gives you the surrounding context.

## Reading Playback Scope

The RG Nano fork shows a tiny playback label on main views:

| Label | Producer meaning |
| --- | --- |
| `STOP` | Transport is stopped. |
| `PLAY:SONG` | You are hearing song/arrangement context across active channels. |
| `PLAY:CHAIN` | You are hearing the current channel's chain context. |
| `PLAY:PHR` | You are hearing the current channel's phrase context. |
| `PLAY:LIVE` | Live/queued chain mode is active. |
| `AUDITION` | You are previewing a note/sample rather than playing the arrangement. |

This matters because a tracker lets you edit deep inside a phrase while still checking the larger arrangement. If the melody sounds wrong alone, use `PLAY:PHR`. If it sounds wrong against drums/bass, use `PLAY:SONG`.
