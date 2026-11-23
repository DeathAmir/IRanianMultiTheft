#pragma once
#include "asset_formats.hpp"
#include <string>

namespace ires {
class Engine {
public:
    bool Initialize(int width, int height, const char* title);
    void BeginFrame(Color clear = Color{18,22,31,255});
    void Begin3D();
    void DrawModel(const ModelAsset& model, Vector3 position, float yaw = 0.0f, float scale = 1.0f);
    void End3D();
    void EndFrame();
    void Shutdown();
    Camera3D& Camera();
    static const char* Version();
private:
    Camera3D camera_{};
};
}
