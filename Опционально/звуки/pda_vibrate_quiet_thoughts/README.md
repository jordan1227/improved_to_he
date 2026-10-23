# PDA vibration, quiet thoughts

Merge this addon's `gamedata` folder over the main mod's `gamedata` folder. Sound files only.

- `pda_news.ogg`: a 0.4 s vibration-motor buzz instead of the Half-Life sample, for every PDA message that uses the `pda_news` sound.
- `pda_thoughts.ogg`: the sound of "Thinking out loud" tips (`sak.present_tip` plays it for every tip with that header unless the caller asked for no sound). Here it is silence (a copy of `$no_sound.ogg`). The main mod ships the Half-Life sample under this name; put any short `.ogg` here to give thoughts their own cue.
