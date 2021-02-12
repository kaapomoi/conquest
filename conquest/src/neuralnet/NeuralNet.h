#pragma once
#include <core/Engine.h>
#include <neuralnet/Neuron.h>

class NeuralNet
{
public:
	NeuralNet(const std::vector<int>& topology);
	void FeedForward(const std::vector<double>& input_values);
	void GetResults(std::vector<double>& result_values) const;


private:
	std::vector<std::vector<Neuron>> layers;
};
