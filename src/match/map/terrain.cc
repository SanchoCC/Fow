#include "terrain.h"

#include <string>

namespace fow {

TerrainModifiers::TerrainModifiers(TerrainType terrain_type) {
  switch (terrain_type) {
    case TerrainType::kPlains:
      attack_bonus = 0.1f;
      defense_bonus = 0.0f;
      break;
    case TerrainType::kHills:
      attack_bonus = 0.3f;
      defense_bonus = 0.2f;
      range_extend = 1;
      movement_cost = 1.5f;
      break;
    case TerrainType::kMountains:
      attack_bonus = 0.1f;
      defense_bonus = 0.3f;
      range_extend = 1;
      movement_cost = 2.f;
      break;
    case TerrainType::kMarsh:
      attack_bonus = 0.f;
      defense_bonus = -0.1f;
      movement_cost = 2.f;
      break;
    case TerrainType::kForest:
      attack_bonus = 0.2f;
      defense_bonus = 0.1f;
      movement_cost = 1.5f;
      break;
    case TerrainType::kWater:
      attack_bonus = -1.f;
      defense_bonus = -0.2f;
      range_extend = -100;
      movement_cost = 2.f;
      break;
    case TerrainType::kUrban:
      break;
    case TerrainType::kRoad:
        movement_cost = 0.8f;
        break;
    case TerrainType::kBridge:
        movement_cost = 0.8f;
        break;
  }
}

Terrain::Terrain(TerrainType terrain_type) : terrain_type_(terrain_type), terrain_modifiers_(terrain_type) {}

std::string TerrainManager::GetName(TerrainType terrain_type) const {
  switch (terrain_type) {
    case TerrainType::kPlains: return "Plain";
    case TerrainType::kHills: return "Hills";
    case TerrainType::kMountains: return "Mountains";
    case TerrainType::kMarsh: return "Marsh";
    case TerrainType::kForest: return "Forest";
    case TerrainType::kWater: return "Water";
    case TerrainType::kUrban: return "Urban";
    case TerrainType::kRoad: return "Road";
    case TerrainType::kBridge: return "Bridge";
  }
}

std::string TerrainManager::GetTextureName(TerrainType terrain_type) const {
  switch (terrain_type) {
    case TerrainType::kPlains: return "terrain/plain/plain";
    case TerrainType::kHills: return "terrain/hills/hills";
    case TerrainType::kMountains: return "terrain/mountains/mountains";
    case TerrainType::kMarsh: return "terrain/marsh/marsh";
    case TerrainType::kForest: return "terrain/forest/forest";
    case TerrainType::kWater: return "terrain/water/water";
    case TerrainType::kUrban: return "terrain/urban/urban";
    case TerrainType::kRoad: return "terrain/road/road";
    case TerrainType::kBridge: return "terrain/bridge/bridge";
  }
}

} // namespace fow