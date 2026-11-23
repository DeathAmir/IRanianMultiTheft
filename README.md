# IRanianMultiTheft

An open-source C++ third-person multiplayer open-world prototype. The project is being built from the ground up and currently includes a procedural test city, third-person movement, a drivable test vehicle, and an ENet-based dedicated server/client foundation.

## Current prototype

- C++17 application
- 3D rendering and input with [raylib](https://github.com/raysan5/raylib) (zlib/libpng)
- UDP networking with [ENet](https://github.com/lsalzman/enet) (MIT)
- Procedurally generated city blocks for the first playable test map
- Third-person player movement and a basic vehicle interaction
- Windows x64 build through GitHub Actions

The current city and character are prototype geometry, not final GTA San Andreas assets. External models and textures should only be added when their licenses explicitly permit redistribution and game use. Prefer CC0 or suitably licensed assets, and keep each asset's attribution/license in `assets/licenses/`.

## Build on Windows

Requirements: Visual Studio 2022 C++ workload, CMake 3.24+, and Git.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
.\build\Release\irantheft.exe
```

CMake downloads raylib and ENet at configure time.

## Run a dedicated server

```powershell
.\build\Release\irantheft.exe --server 27880
```

Allow inbound UDP traffic on port 27880.

## Connect a game client

```powershell
.\build\Release\irantheft.exe --connect 127.0.0.1:27880
```

For a remote server, replace the address with its public IP or hostname. The default client address is `127.0.0.1:27880`.

## Controls

- W/A/S/D: Move or drive
- E: Enter/exit the nearby test car
- Esc: Close the game

## GitHub Actions

Every push to `main`, `develop`, or `feature/**`, every pull request, and manual dispatch runs the Windows x64 build. The executable is uploaded as the `IRanianMultiTheft-Windows-x64` workflow artifact.

## Roadmap

1. Replace prototype geometry with licensed character, vehicle, and environment assets.
2. Add animation controller, collision, and vehicle physics.
3. Add authoritative server-side movement validation and player lifecycle.
4. Add account persistence, spawn selection, chat, and server browser.
5. Expand the city and implement streaming/world partitioning.

## Third-party licenses

- raylib: zlib/libpng license
- ENet: MIT license

See the upstream projects for full license texts.
