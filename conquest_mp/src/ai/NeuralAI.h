#pragma once
#include <core/Engine.h>
#include <ai/AI.h>
#include <neuralnet/NeuralNet.h>
using Random = effolkronium::random_static;

class NeuralAI : public AI
{
public:
	NeuralAI(const char* file_name, std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr);
	virtual ~NeuralAI();


	int Update() override;

	NeuralNet* GetNeuralNet();

	std::vector<int>* GetParentIds() { return &parent_ids; }


	int GetSightSize() { return sight_size; }
	k2d::vi2d GetCurrentVisionPosition() { return vision_grid_position; }


	k2d::vf2d GetPlaystyle() { return k2d::vf2d(x_weight, y_weight); }

	std::vector<int> GetNetTopology() { return net_topology; }

private:
	int try_best;
	std::vector<int> parent_ids;

	std::vector<k2d::vi2d> sc;

	k2d::vi2d vision_grid_position;
	std::vector<k2d::vi2d> end_tiles;
	std::tuple<k2d::vi2d, k2d::vi2d, k2d::vi2d> top3_tiles;

	std::vector<int> net_topology;

	int	sight_size;

	float x_weight;
	float y_weight;

	std::vector<int> fitnesses;

	bool game_won;

	// Random engine
	std::mt19937 rand_engine;

	std::vector<double> result_vec;
	std::vector<std::pair<int, double>> results_map;

	NeuralNet neural_net;

	void get_end_tiles();

	bool valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size);
};