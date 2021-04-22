#pragma once
#include <core/Engine.h>
#include <neuralnet/Neuron.h>

class NeuralNet
{
public:
	NeuralNet(const std::vector<int>& topology);
	NeuralNet(const NeuralNet& other);
	void FeedForward(const std::vector<double>& input_values, std::vector<bool> taken_colors, bool smart);
	void GetResults(std::vector<double>& result_values) const;

	std::vector<std::vector<Neuron>>& GetLayers();

	void SetNet(std::vector<std::vector<Neuron>> net);

private:
	std::vector<std::vector<Neuron>> layers;
};
