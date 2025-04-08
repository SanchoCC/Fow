#pragma once

#include <memory>
#include "map/map.h"
#include "player.h"
#include "targets/units/unit.h"

namespace fow {

	class Match {
	public:
		void InitMap(int rows, int columns);
		void InitPlayers(float basic_width, float basic_height, RCamera2D camera, int number = 2);

		void EndTurn();

		void UpdateRenderMap();

		Map GetMap() const { return *map_; }

		Player& GetCurrentPlayer() { return players_.at(current_player_index_); }
		const UnitManager& GetUnitManager() { return unit_manager_; }
	private:
		void NextPlayer();

		UnitManager unit_manager_;

		std::unique_ptr<Map> map_;
		std::vector<Player> players_;

		int current_player_index_;
	};

}