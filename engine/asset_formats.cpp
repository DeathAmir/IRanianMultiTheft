#include "asset_formats.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;
namespace ires {
bool LoadIrem(const std::string& path, ModelAsset& out) {
    std::ifstream file(path);
    if (!file) return false;
    std::string magic;
    int version = 0;
    file >> magic >> version;
    if (magic != "IREM" || version != 1) return false;
    std::string key;
    while (file >> key) {
        if (key == "name") {
            file >> out.name;
        } else if (key == "box") {
            Primitive p;
            int r=180,g=180,b=190,a=255;
            file >> p.position.x >> p.position.y >> p.position.z
                 >> p.size.x >> p.size.y >> p.size.z >> r >> g >> b >> a;
            p.color = Color{(unsigned char)r,(unsigned char)g,(unsigned char)b,(unsigned char)a};
            out.primitives.push_back(p);
        } else {
            return false;
        }
    }
    return !out.primitives.empty();
}
bool LoadIret(const std::string& path, TextureAsset& out) {
    std::ifstream file(path);
    if (!file) return false;
    std::string magic, kind;
    int version=0,r=255,g=255,b=255,a=255;
    file >> magic >> version >> kind;
    if (magic != "IRET" || version != 1 || kind != "solid") return false;
    file >> r >> g >> b >> a;
    out.color = Color{(unsigned char)r,(unsigned char)g,(unsigned char)b,(unsigned char)a};
    return true;
}
bool SaveIrem(const std::string& path, const ModelAsset& asset) {
    std::ofstream file(path);
    if (!file) return false;
    file << "IREM 1\nname " << (asset.name.empty() ? "Untitled" : asset.name) << "\n";
    for (const auto& p : asset.primitives) {
        file << "box " << p.position.x << ' ' << p.position.y << ' ' << p.position.z << ' '
             << p.size.x << ' ' << p.size.y << ' ' << p.size.z << ' '
             << (int)p.color.r << ' ' << (int)p.color.g << ' ' << (int)p.color.b << ' ' << (int)p.color.a << "\n";
    }
    return file.good();
}
bool PackIremg(const std::string& inputDirectory, const std::string& outputFile) {
    std::vector<std::pair<std::string,std::string>> entries;
    if (!fs::exists(inputDirectory)) return false;
    for (const auto& item : fs::recursive_directory_iterator(inputDirectory)) {
        if (!item.is_regular_file()) continue;
        std::ifstream in(item.path(), std::ios::binary);
        if (!in) continue;
        std::string data((std::istreambuf_iterator<char>(in)), {});
        std::string name = fs::relative(item.path(), inputDirectory).generic_string();
        entries.emplace_back(name, std::move(data));
    }
    std::ofstream out(outputFile, std::ios::binary);
    if (!out) return false;
    const char magic[5] = {'I','R','E','M','G'};
    const uint32_t version=1, count=(uint32_t)entries.size();
    out.write(magic,5); out.write((const char*)&version,4); out.write((const char*)&count,4);
    for (const auto& entry : entries) {
        const uint16_t pathSize=(uint16_t)entry.first.size();
        const uint64_t dataSize=(uint64_t)entry.second.size();
        out.write((const char*)&pathSize,2);
        out.write((const char*)&dataSize,8);
        out.write(entry.first.data(), pathSize);
        out.write(entry.second.data(), (std::streamsize)dataSize);
    }
    return out.good();
}
bool ReadIremg(const std::string& archive, const std::string& path, std::string& contents) {
    std::ifstream in(archive, std::ios::binary);
    if (!in) return false;
    char magic[5]{}; uint32_t version=0,count=0;
    in.read(magic,5); in.read((char*)&version,4); in.read((char*)&count,4);
    if (std::string(magic,5)!="IREMG" || version!=1 || count>100000) return false;
    for (uint32_t i=0;i<count;i++) {
        uint16_t pathSize=0; uint64_t dataSize=0;
        in.read((char*)&pathSize,2); in.read((char*)&dataSize,8);
        if (!in || pathSize>4096 || dataSize>(1ull<<32)) return false;
        std::string name(pathSize,'\0'); in.read(name.data(),pathSize);
        if (!in) return false;
        if (name==path) {
            contents.resize((size_t)dataSize);
            in.read(contents.data(),(std::streamsize)dataSize);
            return in.good() || in.eof();
        }
        in.seekg((std::streamoff)dataSize,std::ios::cur);
        if (!in) return false;
    }
    return false;
}
bool LoadIremgModel(const std::string& archive, const std::string& path, ModelAsset& out) {
    std::string data;
    if (!ReadIremg(archive,path,data)) return false;
    const auto temp=fs::temp_directory_path()/"ires_model_cache.irem";
    { std::ofstream f(temp); if(!f) return false; f << data; }
    const bool ok=LoadIrem(temp.string(),out);
    std::error_code ec; fs::remove(temp,ec);
    return ok;
}
}

namespace ires {
bool LoadIremgTexture(const std::string& archive, const std::string& path, TextureAsset& out) {
    std::string data;
    if (!ReadIremg(archive,path,data)) return false;
    const auto temp=fs::temp_directory_path()/"ires_texture_cache.iret";
    { std::ofstream f(temp); if(!f) return false; f << data; }
    const bool ok=LoadIret(temp.string(),out);
    std::error_code ec; fs::remove(temp,ec);
    return ok;
}
}
