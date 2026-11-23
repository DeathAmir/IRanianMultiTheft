#include "../engine/asset_formats.hpp"
#include <iostream>
int main(int argc,char** argv) {
    if(argc<3) { std::cout << "IRES Asset Packer\nUsage: ires_packer <asset-folder> <output.iremg>\n"; return 0; }
    if(!ires::PackIremg(argv[1],argv[2])) { std::cerr << "Packing failed.\n"; return 1; }
    std::cout << "IREMG archive created: " << argv[2] << "\n";
    return 0;
}
