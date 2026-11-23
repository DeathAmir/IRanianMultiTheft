#include "iresa.hpp"
#include "asset_formats.hpp"
#include "trusted_public_key.hpp"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>

namespace fs=std::filesystem;
namespace ires {
namespace {
constexpr char Magic[5]={'I','R','E','S','A'};
constexpr uint32_t Version=1;
constexpr size_t PrefixSize=18;
constexpr size_t SignatureSize=64;
void Put32(std::string& s,uint32_t v){for(int i=0;i<4;i++)s.push_back((char)(v>>(i*8)));}
void Put64(std::string& s,uint64_t v){for(int i=0;i<8;i++)s.push_back((char)(v>>(i*8)));}
uint32_t Get32(const char* p){uint32_t v=0;for(int i=0;i<4;i++)v|=(uint32_t)(uint8_t)p[i]<<(i*8);return v;}
uint64_t Get64(const char* p){uint64_t v=0;for(int i=0;i<8;i++)v|=(uint64_t)(uint8_t)p[i]<<(i*8);return v;}
bool ReadFile(const std::string& path,std::string& data){
    std::ifstream f(path,std::ios::binary);if(!f)return false;
    data.assign(std::istreambuf_iterator<char>(f),{});return f.good()||f.eof();
}
bool WriteFile(const std::string& path,const std::string& data){
    std::ofstream f(path,std::ios::binary);if(!f)return false;
    f.write(data.data(),(std::streamsize)data.size());return f.good();
}
std::string Prefix(uint8_t type,uint64_t size){
    std::string p(Magic,5);Put32(p,Version);p.push_back((char)type);Put64(p,size);return p;
}
bool VerifySignature(const std::string& data,const unsigned char* key){
    if(data.size()<PrefixSize+SignatureSize||std::memcmp(data.data(),Magic,5)!=0||Get32(data.data()+5)!=Version)return false;
    const uint64_t size=Get64(data.data()+10);
    if(size>(1ull<<32)||data.size()!=PrefixSize+SignatureSize+(size_t)size)return false;
    EVP_PKEY* pub=EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519,nullptr,key,32);
    EVP_MD_CTX* ctx=EVP_MD_CTX_new();bool ok=false;
    const std::string signedBytes=data.substr(0,PrefixSize)+data.substr(PrefixSize+SignatureSize,(size_t)size);
    if(pub&&ctx&&EVP_DigestVerifyInit(ctx,nullptr,nullptr,nullptr,pub)==1)
        ok=EVP_DigestVerify(ctx,(const unsigned char*)data.data()+PrefixSize,SignatureSize,
            (const unsigned char*)signedBytes.data(),signedBytes.size())==1;
    if(ctx)EVP_MD_CTX_free(ctx);if(pub)EVP_PKEY_free(pub);return ok;
}
bool PayloadToModel(const std::string& payload,ModelAsset& out){
    const auto p=fs::temp_directory_path()/"ires_verified_asset.irem";
    if(!WriteFile(p.string(),payload))return false;
    const bool ok=LoadIrem(p.string(),out);std::error_code ec;fs::remove(p,ec);return ok;
}
bool PayloadToTexture(const std::string& payload,TextureAsset& out){
    const auto p=fs::temp_directory_path()/"ires_verified_asset.iret";
    if(!WriteFile(p.string(),payload))return false;
    const bool ok=LoadIret(p.string(),out);std::error_code ec;fs::remove(p,ec);return ok;
}
}
bool SignIresaFile(const std::string& privateKey,const std::string& input,const std::string& output,IresaType type){
    std::string payload;if(!ReadFile(input,payload))return false;
    BIO* bio=BIO_new_file(privateKey.c_str(),"rb");if(!bio)return false;
    EVP_PKEY* key=PEM_read_bio_PrivateKey(bio,nullptr,nullptr,nullptr);BIO_free(bio);
    if(!key||EVP_PKEY_id(key)!=EVP_PKEY_ED25519){if(key)EVP_PKEY_free(key);return false;}
    std::string prefix=Prefix((uint8_t)type,payload.size());
    std::string signedBytes=prefix+payload;
    EVP_MD_CTX* ctx=EVP_MD_CTX_new();unsigned char sig[64];size_t sigLen=sizeof(sig);
    bool ok=ctx&&EVP_DigestSignInit(ctx,nullptr,nullptr,nullptr,key)==1&&
        EVP_DigestSign(ctx,sig,&sigLen,(const unsigned char*)signedBytes.data(),signedBytes.size())==1;
    if(ctx)EVP_MD_CTX_free(ctx);EVP_PKEY_free(key);
    if(!ok||sigLen!=64)return false;
    std::string outputData=prefix;outputData.append((const char*)sig,sigLen);outputData+=payload;
    return WriteFile(output,outputData);
}
bool VerifyIresaData(const std::string& data,IresaType expectedType,std::string& payload){
    if(data.size()<PrefixSize+SignatureSize||!VerifySignature(data,TrustedAssetPublicKey.data()))return false;
    if((uint8_t)data[9]!=(uint8_t)expectedType)return false;
    const uint64_t size=Get64(data.data()+10);
    payload.assign(data.data()+PrefixSize+SignatureSize,(size_t)size);return true;
}
bool VerifyIresaFile(const std::string& path,IresaType expectedType,std::string* payload){
    std::string data,verified;if(!ReadFile(path,data)||!VerifyIresaData(data,expectedType,verified))return false;
    if(payload)*payload=std::move(verified);return true;
}
bool LoadIremgSignedModel(const std::string& archive,const std::string& path,ModelAsset& out){
    std::string data,payload;if(!ReadIremg(archive,path,data)||!VerifyIresaData(data,IresaType::Model,payload))return false;
    return PayloadToModel(payload,out);
}
bool LoadIremgSignedTexture(const std::string& archive,const std::string& path,TextureAsset& out){
    std::string data,payload;if(!ReadIremg(archive,path,data)||!VerifyIresaData(data,IresaType::Texture,payload))return false;
    return PayloadToTexture(payload,out);
}
bool LoadIremgSignedScript(const std::string& archive,const std::string& path,std::string& script){
    std::string data;return ReadIremg(archive,path,data)&&VerifyIresaData(data,IresaType::Script,script);
}
}
