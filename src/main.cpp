#define NOMINMAX
#include "../engine/ires_engine.hpp"
#include "../engine/world.hpp"
#include "raymath.h"
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#define DrawText Win32DrawText
#include <enet/enet.h>
#undef DrawText
#undef ShowCursor
#undef CloseWindow
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>
namespace {
Font gUiFont{};
Model gSoldierModel{};
Model gWeaponModel{};
bool gHasSoldierModel=false;
bool gHasWeaponModel=false;
void UiText(const char* text,int x,int y,float size,Color color) {
 if(gUiFont.texture.id>0) DrawTextEx(gUiFont,text,{(float)x,(float)y},size,1.0f,color);
 else DrawText(text,x,y,(int)size,color);
}
constexpr int DefaultPort=27880;
constexpr uint32_t PacketMagic=0x49524553;
struct NetPacket { uint32_t magic; uint8_t type; uint8_t reserved[3]; uint32_t playerId; float x,z,yaw; };
struct RemotePlayer { Vector3 position{}; float yaw=0; };
struct Target { Vector3 position{}; float health=100.0f; bool active=true; };
int RunServer(int port) {
 if(enet_initialize()!=0) return 1;
 ENetAddress address{}; address.host=ENET_HOST_ANY; address.port=(enet_uint16)port;
 ENetHost* server=enet_host_create(&address,64,2,0,0); if(!server){enet_deinitialize();return 1;}
 std::map<ENetPeer*,uint32_t> peers; std::map<uint32_t,RemotePlayer> players; uint32_t nextId=1;
 auto lastBroadcast=std::chrono::steady_clock::now(); std::printf("IRES multiplayer server listening on UDP %d\n",port); ENetEvent event{};
 while(true) {
  while(enet_host_service(server,&event,5)>0) {
   if(event.type==ENET_EVENT_TYPE_CONNECT) { const uint32_t id=nextId++; peers[event.peer]=id; players[id]={}; event.peer->data=(void*)(uintptr_t)id; std::printf("Player %u connected\n",id); }
   else if(event.type==ENET_EVENT_TYPE_RECEIVE) {
    if(event.packet->dataLength==sizeof(NetPacket)) { NetPacket p{}; std::memcpy(&p,event.packet->data,sizeof(p)); auto it=peers.find(event.peer); if(p.magic==PacketMagic&&p.type==1&&it!=peers.end()) players[it->second]={{p.x,0,p.z},p.yaw}; }
    enet_packet_destroy(event.packet);
   } else if(event.type==ENET_EVENT_TYPE_DISCONNECT) {
    auto it=peers.find(event.peer); if(it!=peers.end()) { NetPacket gone{PacketMagic,3,{0,0,0},it->second,0,0,0}; ENetPacket* packet=enet_packet_create(&gone,sizeof(gone),ENET_PACKET_FLAG_RELIABLE); if(packet) enet_host_broadcast(server,0,packet); players.erase(it->second); peers.erase(it); }
   }
  }
  const auto now=std::chrono::steady_clock::now();
  if(now-lastBroadcast>=std::chrono::milliseconds(50)) { lastBroadcast=now; for(const auto& peer:peers) for(const auto& state:players) { if(state.first==peer.second) continue; NetPacket p{PacketMagic,2,{0,0,0},state.first,state.second.position.x,state.second.position.z,state.second.yaw}; ENetPacket* packet=enet_packet_create(&p,sizeof(p),ENET_PACKET_FLAG_UNSEQUENCED); if(packet) enet_peer_send(peer.first,0,packet); } enet_host_flush(server); }
 }
 enet_host_destroy(server); enet_deinitialize(); return 0;
}
bool CanStand(Vector3 position,const std::vector<ires::Primitive>& world) {
 const float radius=0.42f;
 for(const auto& p:world) {
  if(p.size.y<0.8f||p.position.y<0.0f) continue;
  const float x=Clamp(position.x,p.position.x-p.size.x*0.5f,p.position.x+p.size.x*0.5f);
  const float z=Clamp(position.z,p.position.z-p.size.z*0.5f,p.position.z+p.size.z*0.5f);
  const float dx=position.x-x,dz=position.z-z;
  if(dx*dx+dz*dz<radius*radius) return false;
 }
 return true;
}
void DrawSoldier(Vector3 p,float yaw,bool enemy=false) {
 if(gHasSoldierModel) {
  DrawModelEx(gSoldierModel,{p.x,p.y,p.z},{0,1,0},yaw*RAD2DEG,{1.0f,1.0f,1.0f},WHITE);
  return;
 }
 const Color body=enemy?Color{176,62,54,255}:Color{58,112,166,255};
 DrawCube({p.x,p.y+0.95f,p.z},0.62f,0.95f,0.38f,body);
 DrawCube({p.x,p.y+1.62f,p.z},0.42f,0.42f,0.42f,Color{205,170,137,255});
 DrawCube({p.x-0.18f,p.y+0.30f,p.z},0.19f,0.60f,0.22f,Color{38,43,49,255});
 DrawCube({p.x+0.18f,p.y+0.30f,p.z},0.19f,0.60f,0.22f,Color{38,43,49,255});
 Vector3 forward={sinf(yaw),0,-cosf(yaw)};
 DrawCube(Vector3Add({p.x,p.y+1.18f,p.z},Vector3Scale(forward,0.52f)),0.16f,0.16f,0.82f,Color{32,35,39,255});
}
}
int main(int argc,char** argv) {
 bool serverMode=false; std::string host="127.0.0.1"; int port=DefaultPort;
 for(int i=1;i<argc;i++) { const std::string arg=argv[i]; if(arg=="--server") { serverMode=true; if(i+1<argc) port=std::atoi(argv[++i]); } else if(arg=="--connect"&&i+1<argc) { host=argv[++i]; const auto colon=host.find(':'); if(colon!=std::string::npos){port=std::atoi(host.substr(colon+1).c_str());host.resize(colon);} } }
 if(serverMode) return RunServer(port);
 if(enet_initialize()!=0) return 1;
 ENetHost* client=enet_host_create(nullptr,1,2,0,0); if(!client){enet_deinitialize();return 1;}
 ENetAddress address{}; address.port=(enet_uint16)port; ENetPeer* serverPeer=nullptr; if(enet_address_set_host(&address,host.c_str())==0) serverPeer=enet_host_connect(client,&address,2,0);
 ires::Engine engine; if(!engine.Initialize(1366,768,"IRAntheft | IRES Tactical Shooter")) { enet_host_destroy(client); enet_deinitialize(); return 1; }
 const std::string base=GetApplicationDirectory();
 gUiFont=LoadFontEx((base+"assets/fonts/IBMPlexSans.ttf").c_str(),32,nullptr,0);
 if(gUiFont.texture.id>0) SetTextureFilter(gUiFont.texture,TEXTURE_FILTER_BILINEAR);
 gSoldierModel=LoadModel((base+"assets/models/Swat.glb").c_str());
 gHasSoldierModel=gSoldierModel.meshCount>0;
 gWeaponModel=LoadModel((base+"assets/models/SMG_1.glb").c_str());
 gHasWeaponModel=gWeaponModel.meshCount>0;
 std::vector<ires::Primitive> world;
 if(!ires::LoadMap(base+"assets/maps/arena.iremap",world)) {
  world.push_back({{0,-0.25f,0},{100,0.5f,100},Color{75,91,72,255}});
  for(int i=-5;i<=5;i++) { world.push_back({{(float)i*8,1,-40},{7.8f,2,1},Color{87,99,112,255}}); world.push_back({{(float)i*8,1,40},{7.8f,2,1},Color{87,99,112,255}}); world.push_back({{-40,1,(float)i*8},{1,2,7.8f},Color{87,99,112,255}}); world.push_back({{40,1,(float)i*8},{1,2,7.8f},Color{87,99,112,255}}); }
 }
 std::vector<Target> targets; for(int i=0;i<8;i++) targets.push_back({{(float)((i%4)-2)*7.0f,0,(float)(i/4)*-15.0f-12.0f},100,true});
 bool splash=true,connected=false,reloading=false; float splashTimer=0,sendTimer=0,yaw=0,pitch=0,health=100,shotCooldown=0,reloadTimer=0; int ammo=30,reserve=120,kills=0;
 Vector3 playerPos{0,0,7}; std::map<uint32_t,RemotePlayer> remotes; DisableCursor();
 while(!WindowShouldClose()) {
  const float dt=GetFrameTime(); shotCooldown=std::max(0.0f,shotCooldown-dt);
  ENetEvent event{};
  while(enet_host_service(client,&event,0)>0) {
   if(event.type==ENET_EVENT_TYPE_CONNECT) connected=true;
   else if(event.type==ENET_EVENT_TYPE_DISCONNECT) connected=false;
   else if(event.type==ENET_EVENT_TYPE_RECEIVE) { if(event.packet->dataLength==sizeof(NetPacket)) { NetPacket p{}; std::memcpy(&p,event.packet->data,sizeof(p)); if(p.magic==PacketMagic&&p.type==2) remotes[p.playerId]={{p.x,0,p.z},p.yaw}; if(p.magic==PacketMagic&&p.type==3) remotes.erase(p.playerId); } enet_packet_destroy(event.packet); }
  }
  if(splash) { splashTimer+=dt; if(IsKeyPressed(KEY_ENTER)||splashTimer>2.8f) splash=false; engine.BeginFrame(Color{10,13,22,255}); int w=GetScreenWidth(),h=GetScreenHeight(); DrawCircleGradient(w/2,h/2,220,Color{28,54,89,255},Color{10,13,22,0}); UiText("IRANTHEFT",w/2-230,h/2-95,64,GOLD); UiText("TACTICAL MULTIPLAYER",w/2-175,h/2-15,25,RAYWHITE); UiText("IRES ENGINE",w/2-80,h/2+35,18,Color{110,190,255,255}); UiText("PRESS ENTER TO DEPLOY",w/2-125,h-54,16,Fade(RAYWHITE,0.8f)); engine.EndFrame(); continue; }
  if(IsKeyPressed(KEY_ESCAPE)) break;
  Vector2 mouse=GetMouseDelta(); yaw+=mouse.x*0.0032f; pitch=Clamp(pitch-mouse.y*0.0026f,-0.48f,0.42f);
  Vector3 forward={sinf(yaw),0,-cosf(yaw)},right={cosf(yaw),0,sinf(yaw)},movement={0,0,0};
  if(IsKeyDown(KEY_W)) movement=Vector3Add(movement,forward); if(IsKeyDown(KEY_S)) movement=Vector3Subtract(movement,forward);
  if(IsKeyDown(KEY_D)) movement=Vector3Add(movement,right); if(IsKeyDown(KEY_A)) movement=Vector3Subtract(movement,right);
  if(Vector3Length(movement)>0.01f) {
   Vector3 step=Vector3Scale(Vector3Normalize(movement),(IsKeyDown(KEY_LEFT_SHIFT)?9.0f:5.8f)*dt);
   Vector3 nextX={Clamp(playerPos.x+step.x,-37,37),0,playerPos.z};
   Vector3 nextZ={playerPos.x,0,Clamp(playerPos.z+step.z,-37,37)};
   if(CanStand(nextX,world)) playerPos.x=nextX.x;
   if(CanStand(nextZ,world)) playerPos.z=nextZ.z;
  }
  if(IsKeyPressed(KEY_R)&&ammo<30&&reserve>0&&!reloading) { reloading=true; reloadTimer=1.25f; }
  if(reloading) { reloadTimer-=dt; if(reloadTimer<=0) { int load=std::min(30-ammo,reserve); ammo+=load; reserve-=load; reloading=false; } }
  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&!reloading&&shotCooldown<=0&&ammo>0) {
   ammo--; shotCooldown=0.12f; Vector3 aim={forward.x*cosf(pitch),sinf(pitch),forward.z*cosf(pitch)}; Ray ray={Vector3Add(playerPos,{0,1.55f,0}),aim}; float nearest=1000; int selected=-1;
   for(size_t i=0;i<targets.size();i++) if(targets[i].active) { BoundingBox box={{targets[i].position.x-0.48f,0,targets[i].position.z-0.35f},{targets[i].position.x+0.48f,1.9f,targets[i].position.z+0.35f}}; RayCollision hit=GetRayCollisionBox(ray,box); if(hit.hit&&hit.distance<nearest){nearest=hit.distance;selected=(int)i;} }
   if(selected>=0) { targets[selected].health-=34; if(targets[selected].health<=0) { targets[selected].active=false; kills++; } }
  }
  if(IsKeyPressed(KEY_F)&&health<100) health=std::min(100.0f,health+25.0f);
  sendTimer+=dt; if(connected&&serverPeer&&sendTimer>=0.05f) { sendTimer=0; NetPacket p{PacketMagic,1,{0,0,0},0,playerPos.x,playerPos.z,yaw}; ENetPacket* packet=enet_packet_create(&p,sizeof(p),ENET_PACKET_FLAG_UNSEQUENCED); if(packet) enet_peer_send(serverPeer,0,packet); enet_host_flush(client); }
  Vector3 eye=Vector3Add(playerPos,{0,1.62f,0});
  Vector3 aimDirection={forward.x*cosf(pitch),sinf(pitch),forward.z*cosf(pitch)};
  engine.Camera().position=eye;
  engine.Camera().target=Vector3Add(eye,aimDirection);
  engine.Camera().up={0,1,0};
  engine.Camera().fovy=76.0f;
  engine.BeginFrame(Color{113,158,190,255}); engine.Begin3D(); DrawPlane({0,-0.08f,0},{120,120},Color{67,83,68,255}); DrawGrid(60,2);
  for(const auto& p:world) { DrawCube(p.position,p.size.x,p.size.y,p.size.z,p.color); DrawCubeWires(p.position,p.size.x,p.size.y,p.size.z,Fade(BLACK,0.12f)); }
  for(const auto& t:targets) if(t.active) DrawSoldier(t.position,0.0f,true);
  for(const auto& e:remotes) DrawSoldier(e.second.position,e.second.yaw,true);
  if(gHasWeaponModel) {
   Vector3 weaponPosition=Vector3Add(eye,Vector3Add(Vector3Scale(right,0.28f),Vector3Add(Vector3Scale(aimDirection,0.62f),Vector3{0,-0.25f,0})));
   DrawModelEx(gWeaponModel,weaponPosition,{0,1,0},yaw*RAD2DEG,{0.34f,0.34f,0.34f},WHITE);
  }
  engine.End3D();
  int sw=GetScreenWidth(),sh=GetScreenHeight(); DrawRectangle(18,18,310,112,Fade(Color{9,14,22,255},0.82f)); UiText("IRANTHEFT  /  TACTICAL",32,28,18,GOLD); UiText(connected?"ONLINE  |  CONNECTED":"OFFLINE  |  LOCAL TRAINING",32,55,13,connected?LIME:ORANGE); UiText(TextFormat("KILLS  %02i     PLAYERS  %02i",kills,1+(int)remotes.size()),32,82,15,RAYWHITE);
  DrawRectangle(22,sh-58,220,12,Fade(BLACK,0.6f)); DrawRectangle(22,sh-58,(int)(220*health/100.0f),12,health>35?LIME:RED); UiText(TextFormat("HP %i",(int)health),250,sh-62,17,RAYWHITE);
  DrawRectangle(sw-220,sh-105,196,82,Fade(Color{9,14,22,255},0.82f)); UiText(reloading?"RELOADING":TextFormat("%02i / %03i",ammo,reserve),sw-202,sh-92,30,RAYWHITE); UiText("R RELOAD   SHIFT SPRINT",sw-202,sh-52,11,LIGHTGRAY);
  DrawLine(sw/2-9,sh/2,sw/2+9,sh/2,WHITE); DrawLine(sw/2,sh/2-9,sw/2,sh/2+9,WHITE); UiText("WASD MOVE  MOUSE AIM  LMB FIRE  R RELOAD  F MEDKIT  ESC EXIT",sw/2-260,sh-24,13,RAYWHITE); DrawFPS(sw-82,18); engine.EndFrame();
 }
 EnableCursor();
 if(gHasWeaponModel) UnloadModel(gWeaponModel);
 if(gHasSoldierModel) UnloadModel(gSoldierModel);
 if(gUiFont.texture.id>0) UnloadFont(gUiFont);
 if(serverPeer&&serverPeer->state==ENET_PEER_STATE_CONNECTED) enet_peer_disconnect(serverPeer,0); enet_host_flush(client); engine.Shutdown(); enet_host_destroy(client); enet_deinitialize(); return 0;
}