# Third-party notices and asset provenance

The repository's own source code is licensed under Apache-2.0 (see the root LICENSE). Third-party code and assets retain their own licenses; the root license does not relicense them.

## Runtime and build dependencies

- raylib 5.5 — https://github.com/raysan5/raylib — raylib's zlib-style license; full upstream text is in RAYLIB-LICENSE.txt. Copyright (c) 2013-2024 Ramon Santamaria.
- ENet 1.3.18 — https://github.com/lsalzman/enet — MIT license. Copyright (c) 2002-2024 Lee Salzman.
- OpenSSL 3.x — https://github.com/openssl/openssl — Apache License 2.0. The Windows workflow links the prebuilt openssl-native NuGet distribution; its package license and notices must accompany redistributed OpenSSL binaries.

## 3D characters and animations

- `assets/models/NightStriker.glb` — Quaternius Universal Base Characters, Superhero Male FullBody variant. Source pack: https://quaternius.com/packs/universalbasecharacters.html. License: CC0 1.0 Universal. The official pack states that the rigged humanoid models are compatible with the Universal Animation Library.
- `assets/models/UniversalAnimations.glb` — Quaternius Universal Animation Library. Source pack: https://quaternius.com/packs/universalanimationlibrary.html. License: CC0 1.0 Universal. Contains 120+ retarget-ready humanoid clips, including locomotion and combat/gun animations.
- The binary files are fetched from a public mirror of the Quaternius assets during the Windows build; the original asset creator and license remain Quaternius / CC0. Mirror reference: https://github.com/Seyamalam/blood-league-kickoff/tree/main/public/assets/vendor/quaternius.

## Weapons and fallback character

- `assets/models/Swat.glb` and `assets/models/SMG_1.glb` — Quaternius models from https://github.com/trebeljahr/quaternius-showcase. Original pack pages: https://quaternius.com/packs/ultimatemodularcharacters.html and https://quaternius.com/packs/scifimodularguns.html. License: CC0 1.0 Universal as stated by the original creator for these packs.

## Sound effects

- `assets/audio/gunshot_smg.mp3` — SMG firearm blast from The Free Firearm Sound Library by Ben Jaszczak, Brian Nelson, Kevin Heras and Matthew Nanney. Source: https://opengameart.org/content/the-free-firearm-sound-library. License: CC0. The sound is a blast layer, not a complete multi-layer weapon mix.
- `assets/audio/footstep.ogg` — Kenney RPG Audio, `footstep00.ogg`. Source: https://kenney.nl/assets/rpg-audio. License: CC0 1.0.
- `assets/audio/reload.wav` — `clipload2.wav` by Brian MacIntosh (BMacZero). Source: https://opengameart.org/content/gun-reload-sound-effects. License: CC0; attribution optional.
- `assets/audio/footstep.ogg` and `assets/audio/footstep_alt.ogg` — Kenney RPG Audio, `footstep00.ogg` and `footstep02.ogg`. Source: https://kenney.nl/assets/rpg-audio. License: CC0 1.0.
- `assets/audio/impact.ogg` — Kenney Impact Sounds, `impactMining_002.ogg`. Source: https://kenney.nl/assets/impact-sounds. License: CC0 1.0.
- Sound binaries are fetched during the Windows build from public mirrors so they do not need to be committed as large binary files. The original CC0 terms permit commercial use and redistribution.

## Font

- `assets/fonts/IBMPlexSans.ttf` — IBM Plex Sans, SIL Open Font License 1.1. Source: https://github.com/google/fonts/tree/main/ofl/ibmplexsans. License text: `assets/fonts/OFL-IBM-Plex-Sans.txt`.

The Windows asset-fetch script validates that every required model, animation, font, and sound file exists and exceeds a minimum size before CMake configuration. The game falls back to the existing SWAT model if the animated Quaternius character cannot be loaded; animation playback is enabled only when raylib confirms that the clip skeleton matches the character skeleton.
