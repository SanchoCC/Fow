#pragma once

#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../structs/vector2i.h"
#include "terrain.h"
#include "tile.h"

namespace fow {

struct TerrainDistribution {
  float hills;
  float mountains;
  float marsh;
  float forest;
  float water;
  float urban;
  float plains;
};

class Map {
 public:
  Map(int rows, int columns, TerrainDistribution distribution, int k_cluster);

  std::vector<std::vector<Tile>> GetTiles() const { return tiles_; }
  const TerrainManager& GetTerrainManager() const { return terrain_manager_; }

  Vector2I GetBounds() const;
  std::unordered_set<Vector2I> GetNeighbors(Vector2I position, bool itself = false) const;

  void SetTileFromType(Vector2I position, TerrainType type);
  void GenerateRoads(int max_distance, double chance);

 private:
  void InitSize(int rows, int columns);
  void InitTerrainCompatibility();
  void InitTiles(TerrainDistribution distribution, int k);

  double GetCompatibility(TerrainType a, TerrainType b) const;

  void RandomFillMap(TerrainDistribution distribution, std::mt19937 gen);
  void ClusterTerrains(std::mt19937 gen, int k);
  double CalculateHappiness(Vector2I tile) const;
  std::vector<std::vector<Vector2I>> FindClusters(TerrainType type) const;

  mutable std::unordered_map<Vector2I, std::unordered_set<Vector2I>> neighbors_cache_;

  TerrainManager terrain_manager_;
  std::vector<std::vector<Tile>> tiles_;

  std::unordered_map<TerrainType, std::unordered_map<TerrainType, double>> terrain_compatibility;
};

}  // namespace fow