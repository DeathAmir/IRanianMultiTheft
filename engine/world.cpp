#include "world.hpp"
#include <fstream>
#include <utility>
namespace ires {
bool LoadMap(const std::string& path,std::vector<Primitive>& objects) {
 std::ifstream in(path); if(!in) return false; std::string magic; int version=0; in>>magic>>version; if(magic!="IREMAP"||version!=1) return false;
 std::vector<Primitive> loaded; std::string key;
 while(in>>key) { if(key!="box") return false; Primitive p; int r=180,g=180,b=190,a=255; in>>p.position.x>>p.position.y>>p.position.z>>p.size.x>>p.size.y>>p.size.z>>r>>g>>b>>a; if(!in||p.size.x<=0||p.size.y<=0||p.size.z<=0) return false; p.color=Color{(unsigned char)r,(unsigned char)g,(unsigned char)b,(unsigned char)a}; loaded.push_back(p); }
 objects=std::move(loaded); return true;
}
bool SaveMap(const std::string& path,const std::vector<Primitive>& objects) {
 std::ofstream out(path); if(!out) return false; out<<"IREMAP 1\n";
 for(const auto& p:objects) out<<"box "<<p.position.x<<' '<<p.position.y<<' '<<p.position.z<<' '<<p.size.x<<' '<<p.size.y<<' '<<p.size.z<<' '<<(int)p.color.r<<' '<<(int)p.color.g<<' '<<(int)p.color.b<<' '<<(int)p.color.a<<"\n";
 return out.good();
}
}