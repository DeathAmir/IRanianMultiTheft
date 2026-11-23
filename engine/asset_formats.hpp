#pragma once
#define CloseWindow RaylibCloseWindow
#define ShowCursor RaylibShowCursor
#define DrawText RaylibDrawText
#include "raylib.h"
#undef DrawText
#undef ShowCursor
#undef CloseWindow
#include <cstdint>
#include <string>
#include <vector>

namespace ires {
struct Primitive {
    Vector3 position{};
    Vector3 size{1,1,1};
    Color color{180,180,190,255};
};
struct ModelAsset {
    std::string name;
    std::vector<Primitive> primitives;
};
struct TextureAsset {
    int width = 1;
    int height = 1;
    Color color{255,255,255,255};
};
bool LoadIrem(const std::string& path, ModelAsset& out);
bool LoadIret(const std::string& path, TextureAsset& out);
bool SaveIrem(const std::string& path, const ModelAsset& asset);
bool PackIremg(const std::string& inputDirectory, const std::string& outputFile);
bool ReadIremg(const std::string& archive, const std::string& path, std::string& contents);
bool LoadIremgModel(const std::string& archive, const std::string& path, ModelAsset& out);
bool LoadIremgTexture(const std::string& archive, const std::string& path, TextureAsset& out);
}
