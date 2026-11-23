#include "ires_engine.hpp"
#include "raymath.h"
#include <cmath>

namespace ires {
bool Engine::Initialize(int width,int height,const char* title) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
    InitWindow(width,height,title);
    if (!IsWindowReady()) return false;
    SetTargetFPS(120);
    camera_.position={7,6,9}; camera_.target={0,1,0}; camera_.up={0,1,0};
    camera_.fovy=60; camera_.projection=CAMERA_PERSPECTIVE;
    return true;
}
void Engine::BeginFrame(Color clear) { BeginDrawing(); ClearBackground(clear); }
void Engine::Begin3D() { BeginMode3D(camera_); }
void Engine::DrawModel(const ModelAsset& model,Vector3 position,float yaw,float scale) {
    for (const auto& p:model.primitives) {
        Vector3 local=Vector3Scale(p.position,scale);
        Vector3 rotated={local.x*cosf(yaw)-local.z*sinf(yaw),local.y,local.x*sinf(yaw)+local.z*cosf(yaw)};
        DrawCube(Vector3Add(position,rotated),p.size.x*scale,p.size.y*scale,p.size.z*scale,p.color);
        DrawCubeWires(Vector3Add(position,rotated),p.size.x*scale,p.size.y*scale,p.size.z*scale,Fade(BLACK,0.16f));
    }
}
void Engine::End3D() { EndMode3D(); }
void Engine::EndFrame() { EndDrawing(); }
void Engine::Shutdown() { CloseWindow(); }
Camera3D& Engine::Camera() { return camera_; }
const char* Engine::Version() { return "IRES ENGINE 0.1.0"; }
}
