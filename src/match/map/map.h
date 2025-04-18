#pragma once

#include <vector>

#include "tile.h"
#include "terrain.h"

namespace fow {

    class Map {
    public:
        Map(int rows, int columns);

        std::vector<std::vector<Tile>> GetTiles() const { return tiles_; }
        const TerrainManager& GetTerrainManager() const { return terrain_manager_; }
    private:
        void InitSize(int rows, int columns);
        void InitTiles();

        TerrainManager terrain_manager_;
        std::vector<std::vector<Tile>> tiles_;
    };
}