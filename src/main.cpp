#include "raylib.h"
#include "raymath.h"
#include <enet/enet.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace {
constexpr int DefaultPort = 27880;
constexpr uint32_t PacketMagic = 0x49524D54;

struct NetPacket {
    uint32_t magic;
    uint8_t type;
    uint8_t reserved[3];
    uint32_t playerId;
    float x;
    float z;
    float yaw;
};

struct Player {
    Vector3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f;
    Color color = SKYBLUE;
};

struct RemotePlayer {
    Vector3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f;
};

Color PlayerColor(uint32_t id) {
    static const Color colors[] = { SKYBLUE, ORANGE, LIME, PINK, GOLD, VIOLET, RED };
    return colors[id % (sizeof(colors) / sizeof(colors[0]))];
}

void DrawBuilding(float x, float z, float width, float depth, float height, Color color) {
    DrawCube({ x, height * 0.5f, z }, width, height, depth, color);
    DrawCubeWires({ x, height * 0.5f, z }, width, height, depth, Fade(BLACK, 0.25f));
    for (float y = 2.0f; y < height - 1.0f; y += 2.4f) {
        for (float wx = -width * 0.32f; wx <= width * 0.32f; wx += 2.0f) {
            DrawCube({ x + wx, y, z - depth * 0.501f }, 0.8f, 1.0f, 0.05f, Fade(SKYBLUE, 0.65f));
        }
    }
}

void DrawCity() {
    DrawPlane({ 0, -0.08f, 0 }, { 240, 0.1f, 240 }, Color{ 70, 91, 75, 255 });
    DrawCube({ 0, 0.0f, 0 }, 240, 0.08f, 13, DARKGRAY);
    DrawCube({ 0, 0.0f, 0 }, 13, 0.08f, 240, DARKGRAY);
    for (int i = -100; i <= 100; i += 10) {
        DrawCube({ static_cast<float>(i), 0.015f, 0 }, 0.18f, 0.025f, 5.5f, GOLD);
        DrawCube({ 0, 0.015f, static_cast<float>(i) }, 5.5f, 0.025f, 0.18f, GOLD);
    }

    for (int x = -5; x <= 5; ++x) {
        for (int z = -5; z <= 5; ++z) {
            if (std::abs(x) <= 1 || std::abs(z) <= 1) continue;
            const float bx = static_cast<float>(x * 20);
            const float bz = static_cast<float>(z * 20);
            const float h = 8.0f + static_cast<float>((x * x + z * z * 3 + 17) % 6) * 3.5f;
            const Color c = ((x + z) & 1) ? Color{ 125, 119, 111, 255 } : Color{ 104, 113, 124, 255 };
            DrawBuilding(bx, bz, 12.0f, 12.0f, h, c);
        }
    }
}

void DrawCar(Vector3 p, float yaw, Color color) {
    DrawCube({ p.x, 0.72f, p.z }, 2.2f, 0.8f, 4.0f, color);
    DrawCube({ p.x, 1.28f, p.z - 0.15f }, 1.7f, 0.65f, 2.0f, Color{ 45, 57, 68, 255 });
    DrawCube({ p.x, 0.35f, p.z - 1.25f }, 2.25f, 0.22f, 0.15f, BLACK);
    DrawCube({ p.x, 0.35f, p.z + 1.25f }, 2.25f, 0.22f, 0.15f, BLACK);
    (void)yaw;
}

int RunServer(int port) {
    if (enet_initialize() != 0) {
        std::fprintf(stderr, "ENet initialization failed.\n");
        return 1;
    }
    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = static_cast<enet_uint16>(port);
    ENetHost* server = enet_host_create(&address, 64, 2, 0, 0);
    if (!server) {
        std::fprintf(stderr, "Could not create server on port %d.\n", port);
        enet_deinitialize();
        return 1;
    }

    std::map<ENetPeer*, uint32_t> peerIds;
    std::map<uint32_t, RemotePlayer> players;
    uint32_t nextId = 1;
    std::printf("IRanianMultiTheft dedicated server listening on UDP %d\n", port);
    std::printf("Press Ctrl+C to stop.\n");

    ENetEvent event{};
    while (true) {
        while (enet_host_service(server, &event, 10) > 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) {
                const uint32_t id = nextId++;
                peerIds[event.peer] = id;
                players[id] = {};
                event.peer->data = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
                std::printf("Player %u connected.\n", id);
            } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                if (event.packet->dataLength == sizeof(NetPacket)) {
                    NetPacket packet{};
                    std::memcpy(&packet, event.packet->data, sizeof(packet));
                    auto found = peerIds.find(event.peer);
                    if (packet.magic == PacketMagic && packet.type == 1 && found != peerIds.end()) {
                        auto& p = players[found->second];
                        p.position = { packet.x, 0.0f, packet.z };
                        p.yaw = packet.yaw;
                    }
                }
                enet_packet_destroy(event.packet);
            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                auto found = peerIds.find(event.peer);
                if (found != peerIds.end()) {
                    std::printf("Player %u disconnected.\n", found->second);
                    players.erase(found->second);
                    peerIds.erase(found);
                }
            }
        }

        for (const auto& item : peerIds) {
            ENetPeer* peer = item.first;
            for (const auto& state : players) {
                NetPacket packet{ PacketMagic, 2, { 0, 0, 0 }, state.first,
                                  state.second.position.x, state.second.position.z, state.second.yaw };
                ENetPacket* outgoing = enet_packet_create(&packet, sizeof(packet), ENET_PACKET_FLAG_UNSEQUENCED);
                if (outgoing) enet_peer_send(peer, 1, outgoing);
            }
        }
        enet_host_flush(server);
    }

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}
}

int main(int argc, char** argv) {
    bool serverMode = false;
    std::string host = "127.0.0.1";
    int port = DefaultPort;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--server") {
            serverMode = true;
            if (i + 1 < argc) port = std::atoi(argv[i + 1]);
        } else if (arg == "--connect" && i + 1 < argc) {
            host = argv[++i];
            const auto colon = host.find(':');
            if (colon != std::string::npos) {
                port = std::atoi(host.substr(colon + 1).c_str());
                host.resize(colon);
            }
        }
    }

    if (serverMode) return RunServer(port);

    if (enet_initialize() != 0) {
        std::fprintf(stderr, "ENet initialization failed.\n");
        return 1;
    }

    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        enet_deinitialize();
        return 1;
    }
    ENetAddress address{};
    address.port = static_cast<enet_uint16>(port);
    if (enet_address_set_host(&address, host.c_str()) != 0) {
        std::fprintf(stderr, "Invalid server address: %s\n", host.c_str());
        enet_host_destroy(client);
        enet_deinitialize();
        return 1;
    }
    ENetPeer* serverPeer = enet_host_connect(client, &address, 2, 0);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "IRanianMultiTheft | Open World Multiplayer");
    SetTargetFPS(144);
    Camera3D camera{};
    camera.position = { 0.0f, 7.0f, 12.0f };
    camera.target = { 0.0f, 1.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 65.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Player local;
    std::map<uint32_t, RemotePlayer> remotes;
    bool connected = false;
    bool driving = false;
    Vector3 carPosition{ 4.0f, 0.0f, 5.0f };
    float carYaw = 0.0f;
    float sendTimer = 0.0f;

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.05f);
        ENetEvent event{};
        while (enet_host_service(client, &event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) connected = true;
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT) connected = false;
            else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                if (event.packet->dataLength == sizeof(NetPacket)) {
                    NetPacket packet{};
                    std::memcpy(&packet, event.packet->data, sizeof(packet));
                    if (packet.magic == PacketMagic && packet.type == 2) {
                        remotes[packet.playerId] = { { packet.x, 0.0f, packet.z }, packet.yaw };
                    }
                }
                enet_packet_destroy(event.packet);
            }
        }

        const float turn = (IsKeyDown(KEY_A) ? -1.0f : 0.0f) + (IsKeyDown(KEY_D) ? 1.0f : 0.0f);
        const float forward = (IsKeyDown(KEY_W) ? 1.0f : 0.0f) - (IsKeyDown(KEY_S) ? 1.0f : 0.0f);
        if (IsKeyPressed(KEY_E) && Vector3Distance(local.position, carPosition) < 5.0f) driving = !driving;

        if (driving) {
            carYaw += turn * 2.0f * dt;
            const Vector3 direction{ std::sin(carYaw), 0.0f, std::cos(carYaw) };
            carPosition = Vector3Add(carPosition, Vector3Scale(direction, forward * 13.0f * dt));
            local.position = carPosition;
            local.yaw = carYaw;
        } else {
            local.yaw += turn * 2.5f * dt;
            const Vector3 direction{ std::sin(local.yaw), 0.0f, std::cos(local.yaw) };
            local.position = Vector3Add(local.position, Vector3Scale(direction, forward * 7.0f * dt));
        }

        const Vector3 focus = driving ? carPosition : local.position;
        const Vector3 follow{ -std::sin(local.yaw) * 9.0f, 5.0f, -std::cos(local.yaw) * 9.0f };
        camera.target = Vector3Lerp(camera.target, Vector3Add(focus, { 0, 1.2f, 0 }), 6.0f * dt);
        camera.position = Vector3Lerp(camera.position, Vector3Add(focus, follow), 5.0f * dt);

        sendTimer += dt;
        if (connected && sendTimer >= 0.05f) {
            sendTimer = 0.0f;
            NetPacket packet{ PacketMagic, 1, { 0, 0, 0 }, 0, local.position.x, local.position.z, local.yaw };
            ENetPacket* outgoing = enet_packet_create(&packet, sizeof(packet), ENET_PACKET_FLAG_UNSEQUENCED);
            if (outgoing) enet_peer_send(serverPeer, 0, outgoing);
            enet_host_flush(client);
        }

        BeginDrawing();
        ClearBackground(Color{ 135, 185, 220, 255 });
        BeginMode3D(camera);
        DrawCity();
        DrawCar(carPosition, carYaw, RED);
        if (!driving) {
            DrawCapsule({ local.position.x, 0.45f, local.position.z },
                        { local.position.x, 1.65f, local.position.z }, 0.38f, 8, 8, local.color);
            DrawSphere({ local.position.x, 1.9f, local.position.z }, 0.28f, BEIGE);
        }
        for (const auto& entry : remotes) {
            const auto& p = entry.second;
            DrawCapsule({ p.position.x, 0.45f, p.position.z },
                        { p.position.x, 1.65f, p.position.z }, 0.38f, 8, 8, PlayerColor(entry.first));
            DrawSphere({ p.position.x, 1.9f, p.position.z }, 0.28f, BEIGE);
        }
        EndMode3D();

        DrawRectangle(18, 18, 310, 104, Fade(BLACK, 0.68f));
        DrawText("IRANIAN MULTI THEFT", 32, 30, 20, GOLD);
        DrawText(connected ? "ONLINE  |  Multiplayer connected" : "OFFLINE  |  Connecting to server...", 32, 60, 15, connected ? LIME : ORANGE);
        DrawText(TextFormat("Players nearby: %i", static_cast<int>(remotes.size()) + 1), 32, 84, 15, RAYWHITE);
        DrawText("WASD: Move / Drive     E: Enter or exit car", 20, GetScreenHeight() - 34, 17, RAYWHITE);
        DrawFPS(GetScreenWidth() - 90, 18);
        EndDrawing();
    }

    if (serverPeer && serverPeer->state == ENET_PEER_STATE_CONNECTED) enet_peer_disconnect(serverPeer, 0);
    enet_host_flush(client);
    enet_host_destroy(client);
    enet_deinitialize();
    CloseWindow();
    return 0;
}
