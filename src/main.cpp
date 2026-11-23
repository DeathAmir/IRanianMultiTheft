#include "../engine/ires_engine.hpp"
#include "../engine/iresa.hpp"
#include "raymath.h"
#include <enet/enet.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

namespace {
constexpr int DefaultPort=27880;
constexpr uint32_t PacketMagic=0x49524553;
struct NetPacket {
    uint32_t magic;
    uint8_t type;
    uint8_t reserved[3];
    uint32_t playerId;
    float x,z,yaw;
};
struct RemotePlayer { Vector3 position{}; float yaw=0; };
int RunServer(int port) {
    if(enet_initialize()!=0) return 1;
    ENetAddress address{}; address.host=ENET_HOST_ANY; address.port=(enet_uint16)port;
    ENetHost* server=enet_host_create(&address,64,2,0,0);
    if(!server){enet_deinitialize();return 1;}
    std::map<ENetPeer*,uint32_t> peers;
    std::map<uint32_t,RemotePlayer> players;
    uint32_t nextId=1;
    auto lastBroadcast=std::chrono::steady_clock::now();
    std::printf("IRES multiplayer server listening on UDP %d\n",port);
    ENetEvent event{};
    while(true) {
        while(enet_host_service(server,&event,5)>0) {
            if(event.type==ENET_EVENT_TYPE_CONNECT) {
                const uint32_t id=nextId++;
                peers[event.peer]=id; players[id]={}; event.peer->data=(void*)(uintptr_t)id;
                std::printf("Player %u connected\n",id);
            } else if(event.type==ENET_EVENT_TYPE_RECEIVE) {
                if(event.packet->dataLength==sizeof(NetPacket)) {
                    NetPacket p{}; std::memcpy(&p,event.packet->data,sizeof(p));
                    auto it=peers.find(event.peer);
                    if(p.magic==PacketMagic&&p.type==1&&it!=peers.end()) players[it->second]={{p.x,0,p.z},p.yaw};
                }
                enet_packet_destroy(event.packet);
            } else if(event.type==ENET_EVENT_TYPE_DISCONNECT) {
                auto it=peers.find(event.peer);
                if(it!=peers.end()) {
                    NetPacket gone{PacketMagic,3,{0,0,0},it->second,0,0,0};
                    ENetPacket* packet=enet_packet_create(&gone,sizeof(gone),ENET_PACKET_FLAG_RELIABLE);
                    if(packet) enet_host_broadcast(server,0,packet);
                    players.erase(it->second); peers.erase(it);
                }
            }
        }
        const auto now=std::chrono::steady_clock::now();
        if(now-lastBroadcast>=std::chrono::milliseconds(50)) {
            lastBroadcast=now;
            for(const auto& peer:peers) for(const auto& state:players) {
                if(state.first==peer.second) continue;
                NetPacket p{PacketMagic,2,{0,0,0},state.first,state.second.position.x,state.second.position.z,state.second.yaw};
                ENetPacket* packet=enet_packet_create(&p,sizeof(p),ENET_PACKET_FLAG_UNSEQUENCED);
                if(packet) enet_peer_send(peer.first,0,packet);
            }
            enet_host_flush(server);
        }
    }
    enet_host_destroy(server); enet_deinitialize(); return 0;
}
}
int main(int argc,char** argv) {
    bool serverMode=false;
    std::string host="127.0.0.1";
    int port=DefaultPort;
    for(int i=1;i<argc;i++) {
        const std::string arg=argv[i];
        if(arg=="--server") { serverMode=true; if(i+1<argc) port=std::atoi(argv[++i]); }
        else if(arg=="--connect"&&i+1<argc) {
            host=argv[++i];
            const auto colon=host.find(':');
            if(colon!=std::string::npos){port=std::atoi(host.substr(colon+1).c_str());host.resize(colon);}
        }
    }
    if(serverMode) return RunServer(port);
    if(enet_initialize()!=0) return 1;
    ENetHost* client=enet_host_create(nullptr,1,2,0,0);
    if(!client){enet_deinitialize();return 1;}
    ENetAddress address{}; address.port=(enet_uint16)port;
    ENetPeer* serverPeer=nullptr;
    if(enet_address_set_host(&address,host.c_str())==0) serverPeer=enet_host_connect(client,&address,2,0);

    ires::Engine engine;
    if(!engine.Initialize(1366,768,"IRanianMultiTheft | Powered by IRES ENGINE")) {
        enet_host_destroy(client); enet_deinitialize(); return 1;
    }
    const std::string base=GetApplicationDirectory();
    ires::ModelAsset player,car,building;
    ires::TextureAsset groundMaterial;
    if(!ires::LoadIremgTexture(base+"game.iremg","textures/concrete.iret",groundMaterial)) ires::LoadIret(base+"assets/textures/concrete.iret",groundMaterial);
    if(!ires::LoadIremgModel(base+"game.iremg","models/player.irem",player)) ires::LoadIrem(base+"assets/models/player.irem",player);
    if(!ires::LoadIremgModel(base+"game.iremg","models/sedan.irem",car)) ires::LoadIrem(base+"assets/models/sedan.irem",car);
    if(!ires::LoadIremgModel(base+"game.iremg","models/building.irem",building)) ires::LoadIrem(base+"assets/models/building.irem",building);
    bool splash=true,connected=false,driving=false;
    float splashTimer=0,sendTimer=0,yaw=0,carYaw=0;
    Vector3 playerPos{0,0,0},carPos{4,0,5};
    std::map<uint32_t,RemotePlayer> remotes;
    while(!WindowShouldClose()) {
        const float dt=GetFrameTime();
        ENetEvent event{};
        while(enet_host_service(client,&event,0)>0) {
            if(event.type==ENET_EVENT_TYPE_CONNECT) connected=true;
            else if(event.type==ENET_EVENT_TYPE_DISCONNECT) connected=false;
            else if(event.type==ENET_EVENT_TYPE_RECEIVE) {
                if(event.packet->dataLength==sizeof(NetPacket)) {
                    NetPacket p{}; std::memcpy(&p,event.packet->data,sizeof(p));
                    if(p.magic==PacketMagic&&p.type==2) remotes[p.playerId]={{p.x,0,p.z},p.yaw};
                    if(p.magic==PacketMagic&&p.type==3) remotes.erase(p.playerId);
                }
                enet_packet_destroy(event.packet);
            }
        }
        if(splash) {
            splashTimer+=dt;
            if(IsKeyPressed(KEY_ENTER)||splashTimer>2.8f) splash=false;
            engine.BeginFrame(Color{10,13,22,255});
            const int w=GetScreenWidth(),h=GetScreenHeight();
            DrawCircleGradient(w/2,h/2,220,Color{28,54,89,255},Color{10,13,22,0});
            DrawText("IRES",w/2-130,h/2-105,92,GOLD);
            DrawText("ENGINE",w/2-105,h/2-16,42,RAYWHITE);
            DrawText("IRANIAN MULTI THEFT",w/2-150,h/2+62,20,Color{110,190,255,255});
            DrawRectangle(w/2-170,h/2+112,340,5,Fade(RAYWHITE,0.18f));
            DrawRectangle(w/2-170,h/2+112,(int)(340*Clamp(splashTimer/2.8f,0,1)),5,GOLD);
            DrawText("INITIALIZING WORLD SYSTEMS...",w/2-150,h/2+135,14,LIGHTGRAY);
            DrawText("PRESS ENTER TO CONTINUE",w/2-115,h-54,15,Fade(RAYWHITE,0.75f));
            engine.EndFrame(); continue;
        }
        const float turn=(IsKeyDown(KEY_A)?-1.0f:0.0f)+(IsKeyDown(KEY_D)?1.0f:0.0f);
        const float move=(IsKeyDown(KEY_W)?1.0f:0.0f)-(IsKeyDown(KEY_S)?1.0f:0.0f);
        if(IsKeyPressed(KEY_E)&&Vector3Distance(playerPos,carPos)<4.5f) driving=!driving;
        if(driving) {
            carYaw+=turn*2.0f*dt;
            carPos.x+=sinf(carYaw)*move*11.0f*dt; carPos.z+=cosf(carYaw)*move*11.0f*dt;
            playerPos=carPos;
        } else {
            yaw+=turn*2.4f*dt;
            playerPos.x+=sinf(yaw)*move*6.0f*dt; playerPos.z+=cosf(yaw)*move*6.0f*dt;
        }
        sendTimer+=dt;
        if(connected&&serverPeer&&sendTimer>=0.05f) {
            sendTimer=0;
            NetPacket p{PacketMagic,1,{0,0,0},0,playerPos.x,playerPos.z,driving?carYaw:yaw};
            ENetPacket* packet=enet_packet_create(&p,sizeof(p),ENET_PACKET_FLAG_UNSEQUENCED);
            if(packet) enet_peer_send(serverPeer,0,packet);
            enet_host_flush(client);
        }
        const Vector3 focus=driving?carPos:playerPos;
        engine.Camera().target=Vector3Lerp(engine.Camera().target,{focus.x,1.0f,focus.z},5*dt);
        engine.Camera().position=Vector3Lerp(engine.Camera().position,{focus.x-sinf(yaw)*9,5.5f,focus.z-cosf(yaw)*9},4*dt);
        engine.BeginFrame(Color{135,184,218,255});
        engine.Begin3D();
        DrawPlane(Vector3{0.0f,-0.1f,0.0f},Vector2{240.0f,240.0f},groundMaterial.color); DrawGrid(60,4.0f);
        for(int x=-4;x<=4;x++) for(int z=-4;z<=4;z++) if(abs(x)>=2&&abs(z)>=2) engine.DrawModel(building,{(float)x*18,0,(float)z*18},0,1);
        engine.DrawModel(car,carPos,carYaw,1);
        if(!driving) engine.DrawModel(player,playerPos,yaw,1);
        for(const auto& entry:remotes) engine.DrawModel(player,entry.second.position,entry.second.yaw,1);
        engine.End3D();
        DrawRectangle(18,18,355,132,Fade(BLACK,0.68f));
        DrawText("IRANIAN MULTI THEFT",34,30,22,GOLD);
        DrawText(ires::Engine::Version(),34,59,15,RAYWHITE);
        DrawText(connected?"MULTIPLAYER: CONNECTED":"MULTIPLAYER: OFFLINE",34,83,14,connected?LIME:ORANGE);
        DrawText(TextFormat("Players: %i",1+(int)remotes.size()),34,105,14,RAYWHITE);
        DrawText("WASD: MOVE   E: VEHICLE",22,GetScreenHeight()-34,16,RAYWHITE);
        DrawFPS(GetScreenWidth()-90,18);
        engine.EndFrame();
    }
    if(serverPeer&&serverPeer->state==ENET_PEER_STATE_CONNECTED) enet_peer_disconnect(serverPeer,0);
    enet_host_flush(client); engine.Shutdown(); enet_host_destroy(client); enet_deinitialize();
    return 0;
}
