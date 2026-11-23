# IRanianMultiTheft — IRES ENGINE

A C++17 third-person open-world game foundation powered by **IRES ENGINE**, a small custom 3D engine and asset pipeline built for this project.

## Components

- **IRES Engine** — raylib-backed 3D renderer, camera, scene drawing and runtime.
- **IREM** — project-native 3D model format (v1).
- **IRET** — project-native texture/material format (v1).
- **IREMG** — packed game asset archive (v1).
- **IRES Asset Editor** — lightweight model preview/editor for IREM.
- **IRES Asset Packer** — packs an asset directory into a single IREMG archive.
- Game startup includes an IRES ENGINE splash screen and loading progress.

The initial sample character, sedan and city building are original primitive-based assets created for this repository. External libraries selected for future imports are Quaternius and Kenney; see [asset licensing notes](assets/licenses/ASSETS.md). No GTA/Rockstar files are included.

## Build Windows x64

Requirements: Visual Studio 2022 C++ workload, CMake 3.24+, Git.

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 8
.\build\Release\irantheft.exe
```

CMake downloads raylib at configure time. The Windows workflow builds with eight parallel jobs and uploads the game, editor, packer, packed archive and loose assets.

## Tools

```powershell
.\build\Release\ires_editor.exe assets\models\player.irem
.\build\Release\ires_packer.exe assets game.iremg
```

Editor controls: Left/Right rotate, mouse wheel zoom. The IREM editor opens and previews the model; full mesh editing, animation authoring and material painting are future milestones.

## Game controls

- WASD: move / drive
- E: enter or leave the nearby car
- Enter: skip splash screen
- Esc: exit

## Current scope

This is an engine foundation, not yet a complete GTA-scale engine. Current IREM supports box primitives; IRET supports solid-color materials; IREMG is an uncompressed archive. Advanced mesh import, skeletal animation, physics, lighting/shadows, world streaming, authoritative multiplayer, account services and a full scene editor remain future work.

## Dependencies

- raylib 5.5 — zlib/libpng

See upstream licenses and the repository's asset license notes.
