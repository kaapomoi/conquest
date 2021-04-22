#include <neuralnet/NeuralNet.h>

NeuralNet::NeuralNet()
{

}

NeuralNet::NeuralNet(const std::vector<int>& topology)
{
	int num_layers = topology.size();

	for (int layer_num = 0; layer_num < num_layers; layer_num++)
	{
		// Push back a new layer
		layers.push_back(std::vector<Neuron>());

		// Outputs are 0 if we're in the output layer. They don't actually output anything,
		// we just retrieve their values when we want an output of the net.
		int num_outputs = layer_num == (num_layers - 1) ? 0 : topology[layer_num + 1];

		// Fill the layer with the amount of neurons specified in the topology
		for (int neuron_index = 0; neuron_index < topology[layer_num]; neuron_index++)
		{
			if (layer_num >= num_layers-1)
			{
				layers.back().push_back(Neuron(num_outputs, neuron_index, true));
			}
			else
			{
				layers.back().push_back(Neuron(num_outputs, neuron_index, false));
			}
			//k2d::KUSI_DEBUG("Made a neuron on layer %d!", layer_num);
		}
	}
}

NeuralNet::NeuralNet(const NeuralNet& other)
{
	this->layers = other.layers;
}

void NeuralNet::FeedForward(const std::vector<double>& input_values, std::vector<bool> taken_colors, bool smart)
{
	assert(input_values.size() == layers[0].size());

	// Assign the input values into the input neurons
	for (int i = 0; i < input_values.size(); i++)
	{
		if (!smart)
		{
			layers[0][i].SetOutputValue(input_values[i]);
		}
		else
		{
			int index = input_values[i] + 0.5;
			if (taken_colors[index] == false)
			{
				layers[0][i].SetOutputValue(input_values[i]);
			}
			else
			{
				// "Ignore" taken colors
				layers[0][i].SetOutputValue(-1.0);
			}
		}
		
	}

	// Forward propagation
	for (int layer_num = 1; layer_num < layers.size(); layer_num++)
	{
		std::vector<Neuron>& prev_layer = layers[layer_num - 1];
		for (int n = 0; n < layers[layer_num].size(); n++)
		{
			layers[layer_num][n].FeedForward(prev_layer);
		}
	}

}

void NeuralNet::GetResults(std::vector<double>& result_values) const
{
	result_values.clear();

	for (int n = 0; n < layers.back().size(); n++)
	{
		result_values.push_back(layers.back()[n].GetOutputValue());
	}
}

std::vector<std::vector<Neuron>>& NeuralNet::GetLayers()
{
	return layers;
}

void NeuralNet::SetNet(std::vector<std::vector<Neuron>> net)
{
	layers = net;
}
