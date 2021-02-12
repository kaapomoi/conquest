#pragma once
#include <core/Engine.h>
#include <ai/AI.h>
#include <neuralnet/NeuralNet.h>

class NeuralAI : public AI
{
public:
	NeuralAI(int id, ServerSim* server_sim);
	virtual ~NeuralAI();

	void Update() override;

	void SendInputToServer(int input_num) override;

	void GetTakenColorsFromServer() override;

	Event GetNextEventFromServer() override;

protected:
	// Random engine
	std::mt19937 rand_engine;

	std::vector<double> result_vec;
	std::vector<std::pair<double, int>> results_map;

	NeuralNet neural_net;
};