#pragma once
#include <string>
#include <cstdint>
namespace ires {
enum class IresaType : uint8_t { Model=1, Texture=2, Script=3 };
bool SignIresaFile(const std::string& privateKey,const std::string& input,const std::string& output,IresaType type);
bool VerifyIresaData(const std::string& fileData,IresaType expectedType,std::string& payload);
bool VerifyIresaFile(const std::string& path,IresaType expectedType,std::string* payload=nullptr);
bool LoadIremgSignedModel(const std::string& archive,const std::string& path,struct ModelAsset& out);
bool LoadIremgSignedTexture(const std::string& archive,const std::string& path,struct TextureAsset& out);
bool LoadIremgSignedScript(const std::string& archive,const std::string& path,std::string& script);
}
