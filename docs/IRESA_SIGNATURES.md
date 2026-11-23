# IRESA signed asset format

IRESA v1 wraps one model, texture, or script payload in an Ed25519 signature envelope.

Binary layout (little-endian integers):

1. 5 bytes magic: `IRESA`
2. uint32 version: 1
3. uint8 asset type: 1=model, 2=texture, 3=script
4. uint64 payload byte length
5. 64-byte Ed25519 signature
6. Original payload bytes

The signature authenticates the 18-byte header prefix (magic, version, type and payload length) plus the complete payload. Verification uses the public key compiled into `engine/trusted_public_key.hpp`. Private signing keys must never be committed or distributed with the game.

The game runtime loads model and texture assets only from signed IRESA entries inside IREMG. It has no unsigned fallback. The script helper verifies the signature and returns source bytes; a script VM/execution environment is not yet part of this build.

## MSYS2 UCRT64 setup

Open the **MSYS2 UCRT64** terminal, then install the compiler, CMake, Ninja and OpenSSL:

```bash
pacman -S --needed git mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-openssl
```

From the repository root:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

## Generate your signing key

Run this once:

```bash
./build/ires_signer.exe keygen ires_private.pem engine/trusted_public_key.hpp
```

This creates your private Ed25519 key and writes its matching public key into the engine header. **Never upload, commit, or share `ires_private.pem`.** Back it up offline. The public header is not secret: commit it to your repository, then rebuild:

```bash
cmake --build build -j8
```

The game binaries built after this step trust assets signed by this key. If you generate a different key later, all trusted binaries and signed assets must be rebuilt/re-signed.

## Sign models, textures and scripts

```bash
mkdir -p assets/signed/models assets/signed/textures assets/signed/scripts
./build/ires_signer.exe sign ires_private.pem assets/models/player.irem assets/signed/models/player.iresa model
./build/ires_signer.exe sign ires_private.pem assets/models/sedan.irem assets/signed/models/sedan.iresa model
./build/ires_signer.exe sign ires_private.pem assets/models/building.irem assets/signed/models/building.iresa model
./build/ires_signer.exe sign ires_private.pem assets/textures/concrete.iret assets/signed/textures/concrete.iresa texture
./build/ires_signer.exe sign ires_private.pem scripts/example.lua assets/signed/scripts/example.iresa script
```

The script command is optional; the input script must exist. Pack signed assets into IREMG by re-running CMake configure and building:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

Verify a signed asset against the embedded trusted public key:

```bash
./build/ires_signer.exe verify assets/signed/models/player.iresa model
```

The signer can also be used with MSVC builds from `build/Release/ires_signer.exe`.

Anyone with access to the private key can create assets accepted by binaries containing the corresponding public key. For production, keep the private key in an offline signing environment or protected CI secret, and rotate it if compromised.
