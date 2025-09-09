#pragma once
#include <vector>
#include <memory>
#include <queue>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include "../structs/vector2i.h"
#include "../match/map/map.h"

namespace fow {

	class AStar {
	public:
		struct Node {
			Vector2I pos;
			int parent_arr_index = -1;
			int state = 0; // 0 = unseen, 1 = open, 2 = closed
			int g = 0, h = 0, f = 0;
		};

		Vector2I bounds;

		struct MinByF {
			bool operator()(const Node* a, const Node* b) const {
				if (a->f != b->f) return a->f > b->f;
				return a->pos.x > b->pos.x;
			}
		};

		inline int Idx(int x, int y, int W) {
			return y * W + x;
		}

		static inline int Octile(Vector2I a, Vector2I b) noexcept {
			int dx = std::abs(a.x - b.x);
			int dy = std::abs(a.y - b.y);
			const int D = 10, D2 = 14;
			return D * std::max(dx, dy) + (D2 - D) * std::min(dx, dy);
		}

		void ExpandNeighbors(Node* cur, const Vector2I& goal, int W, int H, std::vector<Node>& nodes, const std::unordered_set<Vector2I>& blocked) {
			int OFFS[8][2] = {
				{ 0,-1},{ 1,-1},{ 1, 0},{ 1, 1},
				{ 0, 1},{-1, 1},{-1, 0},{-1,-1}
			};

			for (int d = 0; d < 8; d++) {
				int nx = cur->pos.x + OFFS[d][0];
				int ny = cur->pos.y + OFFS[d][1];
				PushNode(cur, nx, ny, goal, W, H, nodes, blocked);
			}
		}

		void PushNode(Node* cur, int nx, int ny, const Vector2I& goal, int W, int H, std::vector<Node>& nodes, const std::unordered_set<Vector2I>& blocked) {
			if (!IsInBounds({ nx, ny }, { 0 , 0 }, { W, H })) return;
			if (blocked.contains({ nx, ny })) return;

			int childIndex = Idx(nx, ny, W);
			Node& child = nodes[childIndex];
			if (child.state == 2) return;

			int dx = nx - cur->pos.x;
			int dy = ny - cur->pos.y;
			bool diagonal = (dx != 0 && dy != 0);
			int stepCost = diagonal ? 14 : 10;

			int tentative_g = cur->g + stepCost;

			if (child.state == 0 || tentative_g < child.g) {
				child.parent_arr_index = Idx(cur->pos.x, cur->pos.y, W);
				child.g = tentative_g;
				child.h = Octile({ nx,ny }, goal);
				child.f = child.g + child.h;

				if (child.state != 1) {
					child.state = 1;
				}

				open_set.push(&child);
			}
		}

		std::priority_queue<Node*, std::vector<Node*>, MinByF> open_set;

		std::vector<Vector2I> FindPathAStar(Node start, Vector2I goal, const Map& map_, const std::unordered_set<Vector2I>& blocked) {
			open_set = std::priority_queue<Node*, std::vector<Node*>, MinByF>{};

			bounds = map_.GetBounds();
			int W = bounds.x;
			int H = bounds.y;

			if (!IsInBounds(start.pos, { 0,0 }, { W,H })) return {};
			if (!IsInBounds(goal, { 0,0 }, { W,H })) return {};
			if (start.pos.x == goal.x && start.pos.y == goal.y) return { start.pos };

			std::vector<Node> nodes(W * H);

			int start_idx = Idx(start.pos.x, start.pos.y, W);
			int goal_idx = Idx(goal.x, goal.y, W);

			nodes[start_idx].g = 0;
			nodes[start_idx].h = Octile(start.pos, goal);
			nodes[start_idx].f = nodes[start_idx].h;
			nodes[start_idx].parent_arr_index = -1;
			nodes[start_idx].state = 1; // open

			for (int y = 0; y < H; y++)
				for (int x = 0; x < W; x++) {
					nodes[Idx(x, y, W)].pos = { x,y };
				}

			open_set.push(&nodes[start_idx]);

			while (!open_set.empty()) {
				Node* top = open_set.top();
				open_set.pop();
				if (top->state == 2) continue;
				top->state = 2;

				if (Idx(top->pos.x, top->pos.y, W) == goal_idx) {
					std::vector<Vector2I> path;

					for (int k = goal_idx; k != -1; k = nodes[k].parent_arr_index) {
						path.push_back(nodes[k].pos);
					}

					std::reverse(path.begin(), path.end());
					return path;
				}

				ExpandNeighbors(top, goal, W, H, nodes, blocked);
			}

			return {};
		}

		static inline bool IsInBounds(Vector2I point, Vector2I min, Vector2I max)
		{
			bool result = (point.x >= min.x) && (point.y >= min.y) &&
				(point.x < max.x) && (point.y < max.y);
			return result;
		}

	};
}