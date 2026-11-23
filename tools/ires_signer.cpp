#include "../engine/iresa.hpp"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

static bool Generate(const std::string& privatePath,const std::string& headerPath){
    EVP_PKEY_CTX* ctx=EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519,nullptr);
    EVP_PKEY* key=nullptr;
    bool ok=ctx&&EVP_PKEY_keygen_init(ctx)==1&&EVP_PKEY_keygen(ctx,&key)==1;
    if(ctx)EVP_PKEY_CTX_free(ctx);if(!ok)return false;
    FILE* f=nullptr;
#ifdef _WIN32
    fopen_s(&f,privatePath.c_str(),"wb");
#else
    f=fopen(privatePath.c_str(),"wb");
#endif
    if(!f){EVP_PKEY_free(key);return false;}
    ok=PEM_write_PrivateKey(f,key,nullptr,nullptr,0,nullptr,nullptr)==1;
    fclose(f);
    std::array<unsigned char,32> pub{};size_t n=pub.size();
    ok=ok&&EVP_PKEY_get_raw_public_key(key,pub.data(),&n)==1&&n==32;
    EVP_PKEY_free(key);if(!ok)return false;
    std::ofstream h(headerPath);if(!h)return false;
    h<<"#pragma once\n#include <array>\nnamespace ires {\ninline constexpr std::array<unsigned char,32> TrustedAssetPublicKey = {\n    ";
    for(size_t i=0;i<pub.size();i++){h<<(unsigned)pub[i];if(i+1<pub.size())h<<",";if(i%8==7&&i+1<pub.size())h<<"\n    ";}
    h<<"\n};\n}\n";
    std::cout<<"Ed25519 keypair generated.\nPrivate key: "<<privatePath<<"\nPublic key header: "<<headerPath<<"\nKeep the private key secret and back it up offline. Rebuild IRES Engine after generating the header.\n";
    return h.good();
}
int main(int argc,char** argv){
    if(argc<2){
        std::cout<<"IRES Asset Signer\n"
                 <<"  ires_signer keygen <private.pem> <trusted_public_key.hpp>\n"
                 <<"  ires_signer sign <private.pem> <input-file> <output.iresa> <model|texture|script>\n"
                 <<"  ires_signer verify <file.iresa> <model|texture|script>\n";
        return 0;
    }
    const std::string command=argv[1];
    if(command=="keygen"&&argc==4)return Generate(argv[2],argv[3])?0:1;
    if(command=="sign"&&argc==6){
        ires::IresaType type;
        const std::string t=argv[5];
        if(t=="model")type=ires::IresaType::Model;else if(t=="texture")type=ires::IresaType::Texture;else if(t=="script")type=ires::IresaType::Script;else return 2;
        if(!ires::SignIresaFile(argv[2],argv[3],argv[4],type)){std::cerr<<"Signing failed.\n";return 1;}
        std::cout<<"Signed asset written: "<<argv[4]<<"\n";return 0;
    }
    if(command=="verify"&&argc==4){
        ires::IresaType type;const std::string t=argv[3];
        if(t=="model")type=ires::IresaType::Model;else if(t=="texture")type=ires::IresaType::Texture;else if(t=="script")type=ires::IresaType::Script;else return 2;
        if(!ires::VerifyIresaFile(argv[2],type)){std::cerr<<"INVALID SIGNATURE OR WRONG ASSET TYPE\n";return 1;}
        std::cout<<"Signature OK. Trusted IRESA asset.\n";return 0;
    }
    return 2;
}
