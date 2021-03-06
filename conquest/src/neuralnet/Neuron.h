#pragma once
#include <core/Engine.h>
using Random = effolkronium::random_static;

class Neuron
{
public:
	Neuron(int num_outputs, int my_index);
	~Neuron();

	void FeedForward(const std::vector<Neuron>& prev_layer);

	double TransferFunction(double in);

	
	void SetOutputValue(double val);
	
	double GetOutputValue()const;

	std::vector<double> output_weights;
	double output_value;
	double bias_weight;
	int my_index;
	
	double RandomWeight();
};
