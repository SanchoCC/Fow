#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include <Texture.hpp>

#include "../../drawable/button/texture_button.h"
#include "../../drawable/texture_manager.h"

namespace fow {

    enum class TerrainType {
        kPlains,
        kHills,
        kMountains,
        kMarsh,
        kForest,
        kWater
    };

    class Terrain {
    public:      
        Terrain(TerrainType terrain_type);  

        TerrainType GetType() const { return terrain_type_; }
    private:   
        TerrainType terrain_type_;
    };

    class TerrainManager : public TextureManager<TerrainType, Terrain>{
    private:
        std::string GetTextureName(TerrainType terrain_type) const override;
    };
}