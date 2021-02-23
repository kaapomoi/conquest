#include <neuralnet/Neuron.h>

Neuron::Neuron(int num_outputs, int my_index)
{
	for (int i = 0; i < num_outputs; i++)
	{
		output_weights.push_back(RandomWeight());
	}
	this->my_index = my_index;
	this->bias_weight = RandomWeight();
	//k2d::KUSI_DEBUG(", %f %i!\n", bias_weight, my_index);
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
	output_value = this->bias_weight * TransferFunction(sum);
}

double Neuron::TransferFunction(double in)
{
	//// Rectified linear unit
	/*if (in > 0.0)
	{
		return 1.0;
	}
	else
	{
		return -1.0;
	}*/

	// fast sigmoid
	return in / (1 + abs(in));
}

double Neuron::RandomWeight()
{
	return Random::get(-1.0, 1.0);
}
