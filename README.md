# IRanianMultiTheft — IRES ENGINE

A C++17 third-person open-world multiplayer game foundation powered by **IRES ENGINE**, a custom 3D engine and asset pipeline built for this project.

## Components

- **IRES Engine** — raylib-backed 3D renderer, camera and runtime.
- **IREM** — project-native model format (v1), currently box-based primitive meshes.
- **IRET** — project-native material/texture format (v1), currently solid RGBA materials.
- **IREMG** — packed game archive (v1), stores all asset files in one binary container.
- **IRES Asset Editor** — opens IREM models, adds/removes box parts, rotates, zooms and saves.
- **IRES Asset Packer** — packs the assets directory into game.iremg.
- Branded splash screen and a small third-person multiplayer test world.
- ENet dedicated server/client snapshot replication foundation.

The initial player, sedan and downtown building are original low-poly primitive assets created for this repository. External libraries selected for future imports are Quaternius and Kenney; see [asset licensing notes](assets/licenses/ASSETS.md). Their original pack files have not yet been imported into the repository. No GTA/Rockstar files are included.

## Build Windows x64

Requirements: Visual Studio 2022 C++ workload, CMake 3.24+, Git.

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 8
.\build\Release\irantheft.exe
```

CMake downloads raylib 5.5 and ENet 1.3.18 at configure time. The Windows workflow uses eight parallel build jobs and uploads the game, editor, packer, packed archive and loose assets.

## Run a dedicated server

```powershell
.\build\Release\irantheft.exe --server 27880
```

Allow inbound UDP traffic on port 27880.

## Connect a client

```powershell
.\build\Release\irantheft.exe --connect 127.0.0.1:27880
```

## Tools

```powershell
.\build\Release\ires_editor.exe assets\models\player.irem
.\build\Release\ires_packer.exe assets game.iremg
```

Editor controls: Left/Right rotate, mouse wheel zoom, N adds a box, Delete removes the last box, S saves the IREM file.

## Game controls

- WASD: move / drive
- E: enter or leave the nearby car
- Enter: skip splash screen
- Esc: exit

## Format specifications

See [IRES_FORMATS.md](docs/IRES_FORMATS.md) for IREM, IRET and IREMG v1 layouts.

## Current scope

This is an engine foundation, not yet a complete GTA-scale engine. IREM currently stores box primitives rather than arbitrary vertex/index meshes; IRET stores solid-color materials rather than image maps; IREMG is uncompressed. The editor is a lightweight primitive editor, not a full scene/animation suite. The ENet layer is an early position-snapshot prototype, not production authoritative multiplayer. FBX/glTF import, skeletal animation, physics, lighting/shadows, world streaming, account services, and robust network validation remain future milestones.

## Dependencies

- raylib 5.5 — zlib/libpng
- ENet 1.3.18 — MIT

See upstream licenses and the repository's asset license notes.
