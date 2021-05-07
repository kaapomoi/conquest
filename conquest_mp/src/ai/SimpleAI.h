#pragma once
#include <core/Engine.h>
#include <ai/AI.h>

class SimpleAI : public AI
{
public:
	SimpleAI(std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr);
	virtual ~SimpleAI();

	int Update() override;

	void SetStartingPosition(k2d::vi2d position) override;
	void SetMapSize(k2d::vi2d map_size);
	void SetCurrentColorOwned(int color) override;

private:
	int HandleTurn();
	int bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x, uint8_t y);
	
	bool valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size);

	std::vector<std::pair<int, int>> bfs_amount_of_each_color;

	// Random engine
	std::mt19937 rand_engine;
};