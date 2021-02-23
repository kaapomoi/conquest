#pragma once
#include <core/Engine.h>
#include <ai/AI.h>
#include <neuralnet/NeuralNet.h>
using Random = effolkronium::random_static;

class NeuralAI : public AI
{
public:
	NeuralAI(int id, ServerSim* server_sim);
	NeuralAI(NeuralAI* parent_a, NeuralAI* parent_b, int id, ServerSim* server_sim);
	NeuralAI(const NeuralAI& parent, int id, ServerSim* ss);
	virtual ~NeuralAI();

	void Update() override;

	bool SendInputToServer(int input_num) override;

	void GetTakenColorsFromServer() override;

	Event GetNextEventFromServer() override;

	NeuralNet* GetNeuralNet();

	void Mutate(float mutation_chance);
	void CloseMutate(float mutation_chance, float epsilon);

private:
	int try_best;
	int parent_a_id;
	int parent_b_id;

	// Random engine
	std::mt19937 rand_engine;

	std::vector<double> result_vec;
	std::vector<std::pair<int, double>> results_map;

	NeuralNet neural_net;
};