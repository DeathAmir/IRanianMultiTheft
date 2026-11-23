#include "../engine/ires_engine.hpp"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <string>

int main() {
    ires::Engine engine;
    if(!engine.Initialize(1366,768,"IRanianMultiTheft | Powered by IRES ENGINE")) return 1;
    const std::string base=GetApplicationDirectory();
    ires::ModelAsset player,car,building;
    ires::TextureAsset groundMaterial;
    if(!ires::LoadIremgTexture(base+"game.iremg","textures/concrete.iret",groundMaterial)) ires::LoadIret(base+"assets/textures/concrete.iret",groundMaterial);
    if(!ires::LoadIremgModel(base+"game.iremg","models/player.irem",player)) ires::LoadIrem(base+"assets/models/player.irem",player);
    if(!ires::LoadIremgModel(base+"game.iremg","models/sedan.irem",car)) ires::LoadIrem(base+"assets/models/sedan.irem",car);
    if(!ires::LoadIremgModel(base+"game.iremg","models/building.irem",building)) ires::LoadIrem(base+"assets/models/building.irem",building);
    bool splash=true;
    float splashTimer=0;
    Vector3 playerPos{0,0,0},carPos{4,0,5};
    float yaw=0,carYaw=0;
    bool driving=false;
    while(!WindowShouldClose()) {
        const float dt=GetFrameTime();
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
            engine.EndFrame();
            continue;
        }
        const float turn=(IsKeyDown(KEY_A)?-1.0f:0.0f)+(IsKeyDown(KEY_D)?1.0f:0.0f);
        const float move=(IsKeyDown(KEY_W)?1.0f:0.0f)-(IsKeyDown(KEY_S)?1.0f:0.0f);
        if(IsKeyPressed(KEY_E)&&Vector3Distance(playerPos,carPos)<4.5f) driving=!driving;
        if(driving) {
            carYaw+=turn*2.0f*dt;
            carPos.x+=sinf(carYaw)*move*11.0f*dt;
            carPos.z+=cosf(carYaw)*move*11.0f*dt;
            playerPos=carPos;
        } else {
            yaw+=turn*2.4f*dt;
            playerPos.x+=sinf(yaw)*move*6.0f*dt;
            playerPos.z+=cosf(yaw)*move*6.0f*dt;
        }
        const Vector3 focus=driving?carPos:playerPos;
        engine.Camera().target=Vector3Lerp(engine.Camera().target,{focus.x,1.0f,focus.z},5*dt);
        engine.Camera().position=Vector3Lerp(engine.Camera().position,{focus.x-sinf(yaw)*9,5.5f,focus.z-cosf(yaw)*9},4*dt);
        engine.BeginFrame(Color{135,184,218,255});
        engine.Begin3D();
        DrawPlane({0,-0.1f,0},{240,0.1f,240},groundMaterial.color);
        DrawGrid(60,4.0f);
        for(int x=-4;x<=4;x++) for(int z=-4;z<=4;z++) {
            if(abs(x)<2||abs(z)<2) continue;
            engine.DrawModel(building,{(float)x*18,0,(float)z*18},0,1.0f);
        }
        engine.DrawModel(car,carPos,carYaw,1.0f);
        if(!driving) engine.DrawModel(player,playerPos,yaw,1.0f);
        engine.End3D();
        DrawRectangle(18,18,335,112,Fade(BLACK,0.68f));
        DrawText("IRANIAN MULTI THEFT",34,30,22,GOLD);
        DrawText(ires::Engine::Version(),34,59,15,RAYWHITE);
        DrawText(driving?"DRIVING  |  E: EXIT":"ON FOOT  |  E: ENTER VEHICLE",34,85,14,LIME);
        DrawText("WASD: MOVE     E: VEHICLE",22,GetScreenHeight()-34,16,RAYWHITE);
        DrawFPS(GetScreenWidth()-90,18);
        engine.EndFrame();
    }
    engine.Shutdown();
    return 0;
}
