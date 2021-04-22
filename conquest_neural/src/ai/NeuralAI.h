#pragma once
#include <core/Engine.h>
#include <ai/AI.h>
#include <neuralnet/NeuralNet.h>
using Random = effolkronium::random_static;

class NeuralAI : public AI
{
public:
	NeuralAI(int id, ServerSim* server_sim, int sight_size, std::vector<int> topology);
	NeuralAI(const NeuralAI& parent, int id, ServerSim* ss);
	virtual ~NeuralAI();

	std::vector<NeuralAI*> CrossBreed(NeuralAI* other_parent, int& running_id);

	void Update() override;

	bool SendInputToServer(int input_num) override;

	void GetTakenColorsFromServer() override;

	Event GetNextEventFromServer() override;

	NeuralNet* GetNeuralNet();

	void Mutate(float mutation_chance);
	void CloseMutate(float mutation_chance, float epsilon);
	void MutateTopology(float rate);

	void MutatePlaystyle(float epsilon);
	void MutateSightSize(int epsilon);

	std::vector<int>* GetParentIds() { return &parent_ids; }

	void SetInGame(bool ig)override;

	int GetSightSize() { return sight_size; }
	k2d::vi2d GetCurrentVisionPosition() { return vision_grid_position; }

	void ClearFitnesses();

	k2d::vf2d GetPlaystyle() { return k2d::vf2d(x_weight, y_weight); }

	std::vector<int> GetNetTopology() { return net_topology; }

private:
	int try_best;
	std::vector<int> parent_ids;

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

	int calc_average_fitness();

	bool valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size);
};