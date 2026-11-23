#include "../engine/ires_engine.hpp"
#include "raymath.h"
#include <string>
#include <iostream>
int main(int argc,char** argv) {
    const std::string path=argc>1?argv[1]:"assets/models/player.irem";
    ires::ModelAsset model;
    if(!ires::LoadIrem(path,model)) { std::cerr << "Cannot open IREM model: " << path << "
"; return 1; }
    ires::Engine engine;
    if(!engine.Initialize(1100,720,"IRES ENGINE | Asset Editor")) return 1;
    float yaw=0, zoom=1.0f;
    while(!WindowShouldClose()) {
        const float dt=GetFrameTime();
        if(IsKeyDown(KEY_LEFT)) yaw-=dt;
        if(IsKeyDown(KEY_RIGHT)) yaw+=dt;
        zoom=Clamp(zoom+GetMouseWheelMove()*0.08f,0.25f,3.0f);
        if(IsKeyPressed(KEY_N)) model.primitives.push_back({{0,0,0},{1,1,1},Color{90,170,230,255}});
        if(IsKeyPressed(KEY_DELETE)&&!model.primitives.empty()) model.primitives.pop_back();
        if(IsKeyPressed(KEY_S)) ires::SaveIrem(path,model);
        engine.Camera().position={7/zoom,5/zoom,9/zoom};
        engine.Camera().target={0,1,0};
        engine.BeginFrame(Color{17,20,29,255});
        engine.Begin3D();
        DrawGrid(24,1.0f);
        engine.DrawModel(model,{0,0,0},yaw,1.0f);
        engine.End3D();
        DrawRectangle(0,0,GetScreenWidth(),90,Fade(BLACK,0.72f));
        DrawText("IRES ENGINE  |  ASSET EDITOR",24,12,24,GOLD);
        DrawText(TextFormat("IREM: %s   |   Parts: %i",model.name.c_str(),(int)model.primitives.size()),26,42,15,RAYWHITE);
        DrawText("N: ADD BOX   DEL: REMOVE BOX   S: SAVE   ARROWS: ROTATE   WHEEL: ZOOM",24,66,13,LIGHTGRAY);
        engine.EndFrame();
    }
    engine.Shutdown();
    return 0;
}
