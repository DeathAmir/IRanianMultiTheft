# IRanianMultiTheft — IRES ENGINE

A Windows-first C++17 multiplayer tactical-shooter foundation powered by IRES ENGINE.

## Current playable prototype

- Third-person tactical shooter controls with corrected forward axis: W moves toward world -Z at zero yaw; S moves backward; A/D strafe relative to facing.
- Mouse look, sprint, crosshair aiming, hitscan target practice, ammunition, reload, health and medkit controls.
- Procedural low-poly soldier, rifle, targets, cover and a small enclosed training arena.
- ENet dedicated-server/client position replication prototype.
- Scene-based arena loaded from `assets/maps/arena.iremap`.

## IRES World Editor

Build and run:

```powershell
.\build\Release\ires_map_editor.exe assets\maps\arena.iremap
```

Controls:
- Right mouse drag: orbit camera
- Mouse wheel: zoom
- WASD: pan across the map
- Left click: select an object
- N: add a box at the editor focus
- Delete: remove selected object
- Arrow keys: move selected object
- +/-: resize selected object
- 1 / 2 / 3: change selected object material color
- Ctrl+S: save the map
- Esc: exit

The map format is plain-text `IREMAP 1`. Every `box` record stores position, size and RGBA color. The editor is an early blockout editor; mesh import, terrain sculpting, undo/redo, object hierarchies and collision authoring are not implemented yet.

## Other tools

```powershell
.\build\Release\ires_editor.exe assets\models\player.irem
.\build\Release\ires_packer.exe assets game.iremg
.\build\Release\ires_signer.exe
```

## Build Windows x64

Requirements: Visual Studio 2022 C++ workload, CMake 3.24+, Git.

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 8
.\build\Release\irantheft.exe
```

The build fetches raylib 5.5 and ENet 1.3.18 and links OpenSSL Crypto for IRESA signatures. The GitHub Actions workflow builds on Windows, uploads available intermediate game outputs, and then packages the launcher in a separate job.

## Run a dedicated server

```powershell
.\build\Release\irantheft.exe --server 27880
```

Allow inbound UDP port 27880.

## Connect a client

```powershell
.\build\Release\irantheft.exe --connect 127.0.0.1:27880
```

## Controls

- WASD: move relative to facing
- Mouse: aim / rotate camera
- Left mouse: fire
- R: reload
- Left Shift: sprint
- F: restore 25 health
- Enter: skip splash
- Esc: exit

## Asset and source licenses

The project's own source is Apache-2.0. Third-party code and assets retain their original licenses. See [THIRD_PARTY_NOTICES.md](assets/licenses/THIRD_PARTY_NOTICES.md) for raylib, ENet, OpenSSL and researched CC0 asset sources.

Important: no external character, weapon, font or animation pack has been imported into this commit. The current soldier, rifle, targets and map props are procedural geometry. The game uses raylib's built-in font. Kenney and Quaternius packs are documented as possible CC0 sources, not falsely presented as bundled assets.

## Current limitations

This is a playable gameplay prototype, not a finished commercial shooter. Multiplayer currently replicates player positions only; shooting, health, inventory, match state and anti-cheat are not server-authoritative. Enemy AI, animated imported characters, real weapon models, skeletal animation, audio, advanced lighting, physics, terrain tools, glTF import, world streaming, matchmaking and production network security still need implementation.
