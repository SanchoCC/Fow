#pragma once

#include <string>

#include "../../drawable/texture_manager.h"

namespace fow {

enum class TerrainType {
  kPlains,
  kHills,
  kMountains,
  kMarsh,
  kForest,
  kWater,
  kUrban,
  kRoad,
  kBridge
};

struct TerrainModifiers {
  TerrainModifiers(TerrainType terrain_type);

  int range_extend = 0;
  float attack_bonus = 0.f;
  float defense_bonus = 0.f;
  float movement_cost = 1.f;
};

class Terrain {
 public:
  Terrain(TerrainType terrain_type);

  TerrainType GetType() const { return terrain_type_; }
  TerrainModifiers GetModifiers() const { return terrain_modifiers_; }

 private:
  TerrainType terrain_type_;
  TerrainModifiers terrain_modifiers_;
};

class TerrainManager : public TextureManager<TerrainType, Terrain> {
 public:
  std::string GetName(TerrainType terrain_type) const override;

 private:
  std::string GetTextureName(TerrainType terrain_type) const override;
};

}  // namespace fow