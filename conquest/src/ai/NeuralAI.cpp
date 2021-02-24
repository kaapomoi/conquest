#include <ai/NeuralAI.h>

NeuralAI::NeuralAI(int id, ServerSim* server_sim) :
	AI(id, server_sim), 
	// Set the topology of the net
	neural_net({ (server_sim->GetMapSize().x * server_sim->GetMapSize().y) + (int)server_sim->GetTakenColors().size() + 1,
		90,
		90,
		(int) server_sim->GetTakenColors().size()})
{
	rand_engine.seed(time(NULL));
	tiles_owned = 0;
	parent_a_id = -1;
	parent_b_id = -1;
	try_best = 0;
}
//((server_sim->GetMapSize().x * server_sim->GetMapSize().y) * (int)server_sim->GetTakenColors().size()) + (int)server_sim->GetTakenColors().size() + 1,
NeuralAI::NeuralAI(NeuralAI* parent_a, NeuralAI* parent_b, int id, ServerSim* server_sim):
	AI(id, server_sim),
	neural_net({ (server_sim->GetMapSize().x * server_sim->GetMapSize().y) + (int)server_sim->GetTakenColors().size() + 1,
		90,
		90,
		(int)server_sim->GetTakenColors().size() }),
	parent_a_id(parent_a->GetClientId()),
	parent_b_id(parent_b->GetClientId())
{
	int yep = Random::get(0, 1);

	std::vector<std::vector<Neuron>>& a_layers = parent_a->GetNeuralNet()->GetLayers();
	std::vector<std::vector<Neuron>>& b_layers = parent_b->GetNeuralNet()->GetLayers();

	// Flip parents
	if (yep > 0)
	{
		a_layers = parent_a->GetNeuralNet()->GetLayers();
		b_layers = parent_b->GetNeuralNet()->GetLayers();
	}
	else
	{
		b_layers = parent_a->GetNeuralNet()->GetLayers();
		a_layers = parent_b->GetNeuralNet()->GetLayers();
	}

	std::vector<std::vector<Neuron>> res_net = a_layers;

	double a_dominance = Random::get(0.0, 1.0);

	for (int i = 0; i < a_layers.size(); i++)
	{
		int cutoff = Random::get(0, (int)a_layers[i].size() - 1);

		for (int j = cutoff; j < a_layers[i].size(); j++)
		{
			int num_weights = (int)a_layers[i][j].output_weights.size();
			cutoff = Random::get(0, num_weights - 1);

			// Randomize biases
			res_net[i][j].bias_weight = ((a_layers[i][j].bias_weight * a_dominance) + ((1.0 - a_dominance) * b_layers[i][j].bias_weight)) * 0.5;

			for (size_t k = cutoff; k < num_weights; k++)
			{
				//res_net[i][j].output_weights[k] = b_layers[i][j].output_weights[k];

				// weighted avg 
				res_net[i][j].output_weights[k] = ((a_layers[i][j].output_weights[k] * a_dominance) + ((1.0 - a_dominance) * b_layers[i][j].output_weights[k])) * 0.5;
			}
		}
	}

	GetNeuralNet()->SetNet(res_net);
}

NeuralAI::NeuralAI(const NeuralAI& parent, int id, ServerSim* ss):
	AI(id,ss), neural_net(parent.neural_net)
{
	this->try_best = 0;
	this->games_played = 0;
	this->games_won = 0;
	this->ingame = false;
	this->parent_a_id = parent.client_id;
	this->parent_b_id = -1;
	this->current_turn_players_id = -1;
}

NeuralAI::~NeuralAI()
{

}

void NeuralAI::Update()
{
	if (ingame)
	{
		// Check the server event queue for updates
		Event e = GetNextEventFromServer();

		switch (e.GetType())
		{
		case EventType::GAME_OVER:
		{
			std::string data = e.GetData();

			std::string delimiter = ":";

			size_t pos = 0;
			std::string token;
			std::vector<std::string> tokens;
			while ((pos = data.find(delimiter)) != std::string::npos) {
				token = data.substr(0, pos);
				tokens.push_back(token);
				data.erase(0, pos + delimiter.length());
			}

			int winner_id = (std::stoi(tokens[0]));
			int turns_played = (std::stoi(tokens[1]));
			int match_id = std::stoi(tokens[2]);
			int p0_id = std::stoi(tokens[3]);
			int p1_id = std::stoi(tokens[4]);
			int p0_tiles = std::stoi(tokens[5]);
			int p1_tiles = std::stoi(tokens[6]);
			std::string encoded_turn_history = tokens[7];
			std::string initial_board_state = data;

			if (which_player_am_i == 0 && p0_id == client_id)
			{
				tiles_owned = p0_tiles;
			}
			else if (which_player_am_i == 1 && p1_id == client_id)
			{
				tiles_owned = p1_tiles;
			}

			if (winner_id == client_id)
			{
				AddGameWon();
			}

			AddGamePlayed();
			ingame = false;
			//return;

			break;
		}
		case EventType::TURN_CHANGE:
		{
			std::string data = e.GetData();

			std::string delimiter = ":";

			size_t pos = 0;
			std::string token;
			std::vector<std::string> tokens;
			while ((pos = data.find(delimiter)) != std::string::npos) {
				token = data.substr(0, pos);
				tokens.push_back(token);
				data.erase(0, pos + delimiter.length());
			}

			SetCurrentTurnPlayersId(std::stoi(tokens[0]));

			if (current_turn_players_id == client_id)
			{
				tiles_owned = std::stoi(data);
			}
			try_best = 0;
			break;
		}
		case EventType::INVALID_COLOR:
		{
			try_best++;
			break;
		}
		default:
			break;
		}

		// If its our turn
		if (current_turn_players_id == client_id)
		{
			GetTakenColorsFromServer();

			int res = -1;
			// Send the input values in to the neural net
			std::vector<double> board_state_single_dimension;
			std::vector<std::vector<tile>> board_state = server->GetBoardState();

			// Convert the 2d array into a single dimension array of doubles
			for (size_t i = 0; i < board_state.size(); i++)
			{
				for (size_t j = 0; j < board_state[i].size(); j++)
				{
					double in_value = board_state[i][j].color;
					double min_in_range = 0.0;
					double max_in_range = 1.0 * (taken_colors.size() - 1);
					double min_out_range = -1.0;
					double max_out_range = 1.0;
					double x = (in_value - min_in_range) / (max_in_range - min_in_range);
					double result = min_out_range + (max_out_range - min_out_range) * x;

					//board_state_single_dimension.push_back((double)board_state[i][j].color * 0.1);
					board_state_single_dimension.push_back(result);


					// Different approach, one bit per color, 0.0 or 1.0 
					/*for (size_t i = 0; i < taken_colors.size(); i++)
					{
						board_state_single_dimension.push_back(0.0);
					}*/
					//board_state_single_dimension.at(i * board_state.size() + j + board_state[i][j].color) = 1.0;
				}
			}

			// Players owned colors
			for (size_t i = 0; i < taken_colors.size(); i++)
			{
				// 1 if free, 0.0 if taken
				board_state_single_dimension.push_back(taken_colors.at(i) ? 0.0 : 1.0);
			}

			board_state_single_dimension.push_back(which_player_am_i);

			// Input the board state into the net
			//neural_net.FeedForward(board_state_single_dimension, taken_colors, false);
			neural_net.FeedForward(board_state_single_dimension, taken_colors, false);


			// Get the results from the net
			neural_net.GetResults(result_vec);
			results_map.clear();

			for (size_t i = 0; i < result_vec.size(); i++)
			{
				//std::cout << "res " << i << ": " << result_vec[i] << "\n";
				if (taken_colors.at(i) == false)
				{
					results_map.push_back(std::make_pair(i, result_vec[i]));
				}
			}

			// highest first
			std::sort(results_map.begin(), results_map.end(),
				[](const std::pair<int, double>& a, const std::pair<int, double>& b) -> bool
				{
					return a.second > b.second;
				}
			);



			res = results_map[try_best].first;

			// Send the number to the server
			SendInputToServer(res);
		}
	}
}

bool NeuralAI::SendInputToServer(int input_num)
{
	return server->ReceiveInput(client_id, input_num);
}

void NeuralAI::GetTakenColorsFromServer()
{
	taken_colors = server->GetTakenColors();
}

Event NeuralAI::GetNextEventFromServer()
{
	return server->GetNextEventFromQueue(client_id);
}

NeuralNet* NeuralAI::GetNeuralNet()
{
	return &neural_net;
}

void NeuralAI::Mutate(float mutation_chance)
{
	std::vector<std::vector<Neuron>>& net = GetNeuralNet()->GetLayers();


	for (int i = 0; i < net.size(); i++)
	{
		for (int j = 0; j < net[i].size(); j++)
		{
			int num_weights = (int)net[i][j].output_weights.size();

			float mutate = Random::get(0.0f, 1.0f);

			if (mutate <= mutation_chance)
			{
				net[i][j].bias_weight = Random::get(0.0, 1.0);
			}

			for (size_t k = 0; k < num_weights; k++)
			{
				mutate = Random::get(0.0f, 1.0f);

				if (mutate <= mutation_chance)
				{
					net[i][j].output_weights[k] = Random::get(0.0, 1.0);
				}
			}
		}
	}
}

void NeuralAI::CloseMutate(float mutation_chance, float epsilon)
{
	std::vector<std::vector<Neuron>>& net = GetNeuralNet()->GetLayers();

	for (int i = 0; i < net.size(); i++)
	{
		for (int j = 0; j < net[i].size(); j++)
		{
			int num_weights = (int)net[i][j].output_weights.size();

			float mutate = Random::get(0.0f, 1.0f);

			if (mutate <= mutation_chance)
			{
				net[i][j].bias_weight = net[i][j].bias_weight + Random::get(-epsilon, epsilon);
				k2d::clamp(net[i][j].bias_weight, -1.0, 1.0);
			}

			for (size_t k = 0; k < num_weights; k++)
			{
				mutate = Random::get(0.0f, 1.0f);

				if (mutate <= mutation_chance)
				{
					net[i][j].output_weights[k] = net[i][j].output_weights[k] + Random::get(-epsilon, epsilon);
					k2d::clamp(net[i][j].output_weights[k], -1.0, 1.0);
				}
			}
		}
	}
}