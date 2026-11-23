#pragma once
#include "asset_formats.hpp"
#include <string>
#include <vector>
namespace ires { bool LoadMap(const std::string&, std::vector<Primitive>&); bool SaveMap(const std::string&, const std::vector<Primitive>&); }
