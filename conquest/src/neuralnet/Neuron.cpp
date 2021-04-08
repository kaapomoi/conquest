#include <neuralnet/Neuron.h>

Neuron::Neuron(int num_outputs, int my_index, bool is_output_layer)
{
	for (int i = 0; i < num_outputs; i++)
	{
		output_weights.push_back(RandomWeight());
	}
	this->my_index = my_index;
	this->is_output = is_output_layer;
	// No random biases on output layers nodes
	if (is_output)
	{
		this->bias_weight = 1.0f;
	}
	else
	{
		this->bias_weight = RandomWeight();
	}
}

Neuron::Neuron(const Neuron& other)
{
	this->bias_weight = other.bias_weight;
	this->my_index = other.my_index;
	this->output_value = other.output_value;
	this->output_weights = other.output_weights;
	this->is_output = other.is_output;
}

Neuron::~Neuron()
{

}

void Neuron::SetOutputValue(double val)
{
	output_value = val;
}


double Neuron::GetOutputValue() const
{
	return output_value;
}

void Neuron::FeedForward(const std::vector<Neuron>& prev_layer)
{
	double sum = 0.0;

	// Sum the previous layers output (which are out inputs)
	for (int n = 0; n < prev_layer.size(); n++)
	{
		sum += prev_layer[n].output_value * prev_layer[n].output_weights[my_index];
	}

	// Output the TransferFunction * bias
	if (is_output)
	{
		output_value = TransferFunctionSigmoid(sum);
	}
	else
	{
		output_value = this->bias_weight * TransferFunctionSigmoid(sum);
	}
}

double Neuron::TransferFunctionSigmoid(double in)
{
	// fast sigmoid
	return in / (1 + abs(in));
}

double Neuron::TransferFunctionRELU(double in)
{
	return in;
}

double Neuron::TransferFunctionOnOff(double in)
{
	if (in > 0.0)
	{
		return 1.0;
	}
	else
	{
		return -1.0;
	}
}

double Neuron::RandomWeight()
{
	return Random::get(-1.0, 1.0);
}
