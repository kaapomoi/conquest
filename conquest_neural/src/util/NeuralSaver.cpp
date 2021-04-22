#include <util/NeuralSaver.h>

void NeuralSaver::SaveNeuralAiToFile(const char* file_name, NeuralAI* ai)
{
	int sight_size = ai->GetSightSize();
	float x_weight = ai->GetPlaystyle().x;
	float y_weight = ai->GetPlaystyle().y;

	auto topology = ai->GetNetTopology();

	std::vector<std::vector<Neuron>>& net = ai->GetNeuralNet()->GetLayers();

	std::string playstyle_str = "";
	playstyle_str += std::to_string(sight_size) + " " + std::to_string(x_weight) + " " + std::to_string(y_weight) + " " + std::to_string(net.size());

	
	std::string topology_str = "";
	for (size_t i = 0; i < topology.size(); i++)
	{
		topology_str += std::to_string(topology.at(i)) + " ";
	}


	std::ofstream file(file_name);
	if (!file) {
		k2d::KUSI_ERROR("ERROR::SAVENEURAL::CAN'T OPEN FILE");
	}
	file << playstyle_str << "\n";
	file << topology_str << "\n";

	for (int i = 0; i < net.size(); i++)
	{
		for (int j = 0; j < net[i].size(); j++)
		{
			std::string neuron_data = "";
			int num_weights = (int)net[i][j].output_weights.size();
			Neuron& n = net[i][j];
			neuron_data += std::to_string(n.bias_weight) + " " + std::to_string(n.my_index) + " ";


			for (size_t k = 0; k < num_weights; k++)
			{
				neuron_data += std::to_string(n.output_weights[k]) + " ";
			}
			file << neuron_data << " ";
		}
		file << "\n";
	}

	file.close();
}
