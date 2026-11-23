# IRESA signed asset format

IRESA v1 wraps one model, texture, or script payload in an Ed25519 signature envelope.

Binary layout (little-endian integers):

1. 5 bytes magic: `IRESA`
2. uint32 version: 1
3. uint8 asset type: 1=model, 2=texture, 3=script
4. uint64 payload byte length
5. 64-byte Ed25519 signature
6. Original payload bytes

The signature authenticates the 18-byte header prefix (magic, version, type and payload length) and the complete payload. Verification uses the public key compiled into `engine/trusted_public_key.hpp`. Private signing keys must never be committed or distributed with the game.

The game runtime accepts model, texture and script payloads from IREMG only through the signed IRESA verification functions. The script helper returns verified source bytes; this repository does not yet include a script VM/execution environment.

## Key generation

Build `ires_signer`, then:

```bash
./build/ires_signer keygen ires_private.pem engine/trusted_public_key.hpp
```

This writes a private Ed25519 PEM key and replaces the trusted public-key header. Rebuild the game/editor/signer after generating the header. Keep the private key offline and back it up securely.

## Signing

```bash
./build/ires_signer sign ires_private.pem assets/models/player.irem assets/signed/models/player.iresa model
./build/ires_signer sign ires_private.pem assets/textures/concrete.iret assets/signed/textures/concrete.iresa texture
./build/ires_signer sign ires_private.pem scripts/example.lua assets/signed/scripts/example.iresa script
```

Pack the `assets` directory into IREMG after signing. The game expects signed entries under `signed/models` and `signed/textures`. Do not package the private key.

If you rotate the key, regenerate the public header, rebuild all trusted binaries, and re-sign every asset. Anyone with access to the private key can create assets accepted by the corresponding public-key build.
