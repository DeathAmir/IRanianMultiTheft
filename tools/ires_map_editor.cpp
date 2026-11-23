#include "../engine/ires_engine.hpp"
#include "../engine/world.hpp"
#include "raymath.h"
#include <algorithm>
#include <string>
#include <vector>
int main(int argc,char** argv) {
 const std::string path=argc>1?argv[1]:"assets/maps/arena.iremap"; std::vector<ires::Primitive> objects;
 if(!ires::LoadMap(path,objects)) {
  objects.push_back({{0,-0.25f,0},{100,0.5f,100},Color{75,91,72,255}});
  for(int i=-5;i<=5;i++) { objects.push_back({{(float)i*8,1,-40},{7.8f,2,1},Color{87,99,112,255}}); objects.push_back({{(float)i*8,1,40},{7.8f,2,1},Color{87,99,112,255}}); objects.push_back({{-40,1,(float)i*8},{1,2,7.8f},Color{87,99,112,255}}); objects.push_back({{40,1,(float)i*8},{1,2,7.8f},Color{87,99,112,255}}); }
 }
 ires::Engine engine; if(!engine.Initialize(1280,800,"IRES WORLD EDITOR")) return 1;
 Vector3 target{0,0,0}; float orbit=0.78f,pitch=0.55f,distance=34; int selected=-1; bool dirty=false; DisableCursor();
 while(!WindowShouldClose()) {
  float dt=GetFrameTime();
  if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) { Vector2 d=GetMouseDelta(); orbit-=d.x*0.004f; pitch=Clamp(pitch+d.y*0.004f,0.12f,1.35f); }
  distance=Clamp(distance-GetMouseWheelMove()*2.0f,8,100);
  engine.Camera().position={target.x+sinf(orbit)*cosf(pitch)*distance,target.y+sinf(pitch)*distance,target.z+cosf(orbit)*cosf(pitch)*distance}; engine.Camera().target=target;
  if(IsKeyDown(KEY_W)) target.z-=18*dt; if(IsKeyDown(KEY_S)) target.z+=18*dt; if(IsKeyDown(KEY_A)) target.x-=18*dt; if(IsKeyDown(KEY_D)) target.x+=18*dt;
  if(IsKeyPressed(KEY_N)) { objects.push_back({{target.x,1,target.z},{2,2,2},Color{78,145,205,255}}); selected=(int)objects.size()-1; dirty=true; }
  if(IsKeyPressed(KEY_DELETE)&&selected>=0&&selected<(int)objects.size()) { objects.erase(objects.begin()+selected); selected=-1; dirty=true; }
  if(selected>=0&&selected<(int)objects.size()) { auto& p=objects[selected];
   if(IsKeyDown(KEY_LEFT)) p.position.x-=8*dt; if(IsKeyDown(KEY_RIGHT)) p.position.x+=8*dt; if(IsKeyDown(KEY_UP)) p.position.z-=8*dt; if(IsKeyDown(KEY_DOWN)) p.position.z+=8*dt;
   if(IsKeyPressed(KEY_EQUAL)||IsKeyPressed(KEY_KP_ADD)) p.size=Vector3Add(p.size,{0.5f,0.5f,0.5f});
   if(IsKeyPressed(KEY_MINUS)||IsKeyPressed(KEY_KP_SUBTRACT)) p.size={std::max(0.25f,p.size.x-0.5f),std::max(0.25f,p.size.y-0.5f),std::max(0.25f,p.size.z-0.5f)};
   if(IsKeyPressed(KEY_ONE)) p.color=Color{78,145,205,255}; if(IsKeyPressed(KEY_TWO)) p.color=Color{170,78,65,255}; if(IsKeyPressed(KEY_THREE)) p.color=Color{95,150,95,255}; dirty=true;
  }
  if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { Ray ray=GetMouseRay(GetMousePosition(),engine.Camera()); float best=100000; selected=-1;
   for(size_t i=0;i<objects.size();i++) { auto& p=objects[i]; BoundingBox b={{p.position.x-p.size.x/2,p.position.y-p.size.y/2,p.position.z-p.size.z/2},{p.position.x+p.size.x/2,p.position.y+p.size.y/2,p.position.z+p.size.z/2}}; RayCollision hit=GetRayCollisionBox(ray,b); if(hit.hit&&hit.distance<best){best=hit.distance;selected=(int)i;} }
  }
  if(IsKeyPressed(KEY_S)&&IsKeyDown(KEY_LEFT_CONTROL)) dirty=!ires::SaveMap(path,objects);
  if(IsKeyPressed(KEY_ESCAPE)) break;
  engine.BeginFrame(Color{18,23,32,255}); engine.Begin3D(); DrawPlane({0,-0.03f,0},{200,200},Color{55,68,62,255}); DrawGrid(100,2);
  for(size_t i=0;i<objects.size();i++) { const auto& p=objects[i]; DrawCube(p.position,p.size.x,p.size.y,p.size.z,p.color); DrawCubeWires(p.position,p.size.x,p.size.y,p.size.z,(int)i==selected?YELLOW:Fade(BLACK,0.35f)); }
  engine.End3D(); DrawRectangle(0,0,GetScreenWidth(),112,Fade(BLACK,0.82f)); DrawText("IRES WORLD EDITOR",22,12,26,GOLD); DrawText(TextFormat("OBJECTS %i   SELECTED %i   %s",(int)objects.size(),selected+1,dirty?"UNSAVED":"SAVED"),24,46,16,RAYWHITE); DrawText("RMB ORBIT  WHEEL ZOOM  WASD PAN  CLICK SELECT  N ADD  DEL REMOVE  ARROWS MOVE  +/- SIZE  1-3 COLOR  CTRL+S SAVE",24,78,13,LIGHTGRAY); DrawText(path.c_str(),24,GetScreenHeight()-26,13,GRAY); engine.EndFrame();
 }
 EnableCursor(); engine.Shutdown(); return 0;
}