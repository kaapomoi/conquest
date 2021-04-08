#include <ai/NeuralAI.h>

NeuralAI::NeuralAI(int id, ServerSim* server_sim, int sight_size, std::vector<int> topology) :
	AI(id, server_sim), 
	// Set the topology of the net
	neural_net(topology)
{
	rand_engine.seed(time(NULL));
	tiles_owned = 0;
	try_best = 0;
	this->sight_size = sight_size;
	x_weight = Random::get(0.0f, 1.0f);
	y_weight = Random::get(0.0f, 1.0f);
	fitness = 0;
	game_won = false;
}

NeuralAI::NeuralAI(const NeuralAI& parent, int id, ServerSim* ss):
	AI(id,ss), neural_net(parent.neural_net)
{
	this->try_best = 0;
	this->games_played = 0;
	this->games_won = 0;
	this->fitness = 0;
	this->ingame = false;
	this->game_won = false;
	this->x_weight = parent.x_weight;
	this->y_weight = parent.y_weight;
	this->sight_size = parent.sight_size;
	this->parent_ids = parent.parent_ids;
	this->parent_ids.push_back(parent.client_id);
	this->current_turn_players_id = -1;
}

NeuralAI::~NeuralAI()
{

}

std::vector<NeuralAI*> NeuralAI::CrossBreed(NeuralAI* other_parent, int& running_id)
{
	NeuralAI* a = new NeuralAI(*this);
	NeuralAI* b = new NeuralAI(*other_parent);
	a->parent_ids.push_back(this->client_id);
	a->parent_ids.push_back(other_parent->client_id);

	b->parent_ids.push_back(this->client_id);
	b->parent_ids.push_back(other_parent->client_id);

	a->client_id = running_id++;
	b->client_id = running_id++;

	std::vector<std::vector<Neuron>>& net_a = a->GetNeuralNet()->GetLayers();

	std::vector<std::vector<Neuron>>& net_b = b->GetNeuralNet()->GetLayers();

	for (size_t layer = 0; layer < net_a.size(); layer++)
	{
		int crossover_index = Random::get(1, (int) net_a[layer].size() - 2);
		for (size_t neuron_index = 0; neuron_index < net_a[layer].size(); neuron_index++)
		{
			// Cross the neurons 
			if (neuron_index >= crossover_index)
			{
				Neuron tmp_neuron = net_a[layer][neuron_index];
				net_a[layer][neuron_index] = net_b[layer][neuron_index];
				net_b[layer][neuron_index] = tmp_neuron;
			}
		}
	}

	std::vector<NeuralAI*> children;

	children.push_back(a);
	children.push_back(b);

	return children;
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
				//SetFitness(tiles_owned + 250 - ((turns_played - 50) * 5));

				AddGameWon();
			}
			else
			{
				fitnesses.push_back(tiles_owned - ((turns_played - 50) * 2));
				SetFitness(calc_average_fitness());
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

				if (tiles_owned > (server->GetMapSize().x * server->GetMapSize().y * 0.5f) && !game_won)
				{
					// Game won, calculate fitness
					fitnesses.push_back(tiles_owned + ((100 - server->GetTurnsPlayed()) * 5));
					SetFitness(calc_average_fitness());
					game_won = true;
				}
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


			/// calculate the furthest tile from starting pos and place the middle of the sight rect on that tile
			get_end_tiles();
			k2d::vi2d best_tile;

			float best_score = 0.0f;
			k2d::vi2d start_pos = server->GetStartingPositions()[which_player_am_i];
			std::sort(end_tiles.begin(), end_tiles.end(), [start_pos, this](const k2d::vi2d a, const k2d::vi2d b) -> bool
				{
					float dx_a = a.x - start_pos.x;
					float dy_a = a.y - start_pos.y;
					float score_a = abs(dx_a) * x_weight + abs(dy_a) * y_weight;
					float dx_b = b.x - start_pos.x;
					float dy_b = b.y - start_pos.y;
					float score_b = abs(dx_b) * x_weight + abs(dy_b) * y_weight;
					return score_a > score_b;
				});

			/*for (size_t i = 0; i < end_tiles.size(); i++)
			{
				float dx = end_tiles.at(i).x - server->GetStartingPositions()[which_player_am_i].x;
				float dy = end_tiles.at(i).y - server->GetStartingPositions()[which_player_am_i].y;
				float score = abs(dx) * x_weight + abs(dy) * y_weight;
				if (score >= best_score)
				{
					best_tile = end_tiles.at(i);
				}
			}*/
			//Random::get(0, (int) end_tiles.size()-1)
			std::vector<float> weights;
			for (size_t i = 0; i < end_tiles.size(); i++)
			{
				weights.push_back(100 / (float) (i + 1));
			}
			std::discrete_distribution<int> distribution(weights.begin(), weights.begin() + end_tiles.size());
			std::mt19937 r_engine;

			best_tile = end_tiles.at(distribution(r_engine));

			vision_grid_position = best_tile;
			k2d::vi2d& t = vision_grid_position;
			int s_h_l = sight_size / 2;
			int s_h_r = sight_size - s_h_l;

			int margin_left = t.x - s_h_l;
			int margin_right = board_state[0].size() - (t.x + s_h_r);
			int margin_bot = t.y - s_h_l;
			int margin_top = board_state.size() - (t.y + s_h_r);

			if (margin_left < 0)
			{
				t.x -= margin_left;
			}
			if (margin_right < 0)
			{
				t.x += margin_right;
			}
			if (margin_bot < 0)
			{
				t.y -= margin_bot;
			}
			if (margin_top < 0)
			{
				t.y += margin_top;
			}


			//// Convert the 2d array into a single dimension array of doubles
			for (size_t i = t.y - s_h_l; i < t.y + s_h_r; i++)
			{
				for (size_t j = t.x - s_h_l; j < t.x + s_h_r; j++)
				{
					double in_value = board_state[i][j].color;
					double min_in_range = 0.0;
					double max_in_range = 1.0 * (taken_colors.size() - 1);
					double min_out_range = -1.0;
					double max_out_range = 1.0;

					double result = k2d::map_to_range(in_value, min_in_range, max_in_range, min_out_range, max_out_range);


					board_state_single_dimension.push_back(result);

					//uint8_t owner = board_state[i][j].owner;
					//if (owner == -9)
					//{
					//	// this tile is neutral
					//	board_state_single_dimension.push_back(-1);
					//}
					//else if (owner != which_player_am_i)
					//{
					//	// Opponent owns this tile
					//	board_state_single_dimension.push_back(-1);
					//}
					//else
					//{
					//	// We own this tile
					//	board_state_single_dimension.push_back(1);
					//}



					// Different approach, one bit per color, 0.0 or 1.0 
					/*for (size_t i = 0; i < taken_colors.size(); i++)
					{
						board_state_single_dimension.push_back(0.0);
					}*/
					//board_state_single_dimension.at(i * board_state.size() + j + board_state[i][j].color) = 1.0;
				}
			}


			//// Convert the 2d array into a single dimension array of doubles
			//for (size_t i = 0; i < board_state.size(); i++)
			//{
			//	for (size_t j = 0; j < board_state[i].size(); j++)
			//	{
			//		double in_value = board_state[i][j].color;
			//		double min_in_range = 0.0;
			//		double max_in_range = 1.0 * (taken_colors.size() - 1);
			//		double min_out_range = -1.0;
			//		double max_out_range = 1.0;
			//		double x = (in_value - min_in_range) / (max_in_range - min_in_range);
			//		double result = min_out_range + (max_out_range - min_out_range) * x;

			//		//board_state_single_dimension.push_back((double)board_state[i][j].color * 0.1);
			//		board_state_single_dimension.push_back(result);


			//		// Different approach, one bit per color, 0.0 or 1.0 
			//		/*for (size_t i = 0; i < taken_colors.size(); i++)
			//		{
			//			board_state_single_dimension.push_back(0.0);
			//		}*/
			//		//board_state_single_dimension.at(i * board_state.size() + j + board_state[i][j].color) = 1.0;
			//	}
			//}

			// Players owned colors
			for (size_t i = 0; i < taken_colors.size(); i++)
			{
				// 1 if free, 0.0 if taken
				board_state_single_dimension.push_back(taken_colors.at(i) ? 0.0 : 1.0);
			}

			//board_state_single_dimension.push_back(which_player_am_i);

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

void NeuralAI::MutateTopology(float rate)
{
	std::vector<std::vector<Neuron>>& net = GetNeuralNet()->GetLayers();

	// should only affect hidden layers
	for (int i = 1; i < net.size() - 1; i++)
	{
		for (int j = 0; j < net[i].size(); j++)
		{
			float r = Random::get(0.0f, 1.0f);
			if (r < rate)
			{
				// Erase node if we hit it
				net[i].erase(net[i].begin() + j);
				// Move my_index of this layers nodes one back	
				for (size_t k = j; k < net[i].size(); k++)
				{
					net[i].at(k).my_index = net[i].at(k).my_index - 1;
				}

				// Remove the output weights of the previous layer connected to this neuron
				for (size_t k = 0; k < net[i-1].size(); k++)
				{
					net[i - 1].at(k).output_weights.erase(net[i - 1].at(k).output_weights.begin() + j);
				}

			}
			else if (r > (1.0f - rate))
			{
				//net[i].push_back(Neuron(net[i+1].size(), ));
				net[i].insert(net[i].begin() + j, Neuron(net[i + 1].size(), j, false));

				// Move my_index of this layers nodes one forward	
				for (size_t k = j + 1; k < net[i].size(); k++)
				{
					net[i].at(k).my_index = net[i].at(k).my_index + 1;
				}

				// Remove the output weights of the previous layer connected to this neuron
				for (size_t k = 0; k < net[i - 1].size(); k++)
				{
					net[i - 1].at(k).output_weights.insert(net[i - 1].at(k).output_weights.begin()+ j, Random::get(-1.0, 1.0));
				}
			}
		}
	}
}

void NeuralAI::MutatePlaystyle(float epsilon)
{
	x_weight += Random::get(-epsilon, epsilon);
	k2d::clamp(x_weight, 0.0f, 1.0f);

	y_weight += Random::get(-epsilon, epsilon);
	k2d::clamp(y_weight, 0.0f, 1.0f);
}

void NeuralAI::MutateSightSize(int epsilon)
{
	std::vector<std::vector<Neuron>>& net = GetNeuralNet()->GetLayers();
	std::vector<std::vector<Neuron>> old_2d_first_layer;
	std::vector<std::vector<Neuron>> new_first_layer;
	int num_outputs = net[1].size();

	// Mutate the sight size variable
	int old_size = sight_size;

	// save old layer
	for (int y = 0; y < old_size; y++)
	{
		old_2d_first_layer.push_back(std::vector<Neuron>());
		for (int x = 0; x < old_size * 2; x++)
		{
			old_2d_first_layer[y].push_back(net[0].at(y * old_size * 2 + x));
		}
	}
	old_2d_first_layer.push_back(std::vector<Neuron>());
	for (size_t i = 0; i < server->GetTakenColors().size(); i++)
	{
		old_2d_first_layer[old_size].push_back(net[0].at(old_size * old_size *2 + i));
	}

	sight_size += Random::get(-epsilon, epsilon);

	k2d::clamp(sight_size, 3, server->GetMapSize().y);
	int diff = sight_size - old_size;

	int num_of_new_neurons = sight_size * sight_size * 2 + server->GetTakenColors().size();

	// Fill the layer with the amount of neurons specified in the topology
	for (int y = 0; y < sight_size; y++)
	{
		new_first_layer.push_back(std::vector<Neuron>());
		for (int x = 0; x < sight_size * 2; x++)
		{
			new_first_layer[y].push_back(Neuron(num_outputs, y * sight_size * 2 + x, false));
		}
	}
	std::vector<Neuron> new_1d_layer;
	if (diff > 0)
	{
		if (diff % 2 == 0)
		{
			// Fill the layer with the amount of neurons specified in the topology
			for (int y = diff / 2; y < old_2d_first_layer.size() - (diff / 2); y++)
			{
				for (int x = diff / 2; x < old_2d_first_layer[y].size() - (diff / 2); x++)
				{
					new_first_layer[y][x] = old_2d_first_layer[y][x];
				}
			}
		}
		else
		{
			// Fill the layer with the amount of neurons specified in the topology
			for (int y = ceil((diff+1) / 2); y < old_2d_first_layer.size() - ((diff-1) / 2); y++)
			{
				for (int x = ceil((diff+1) / 2); x < old_2d_first_layer[y].size() - ((diff-1) / 2); x++)
				{
					new_first_layer[y][x] = old_2d_first_layer[y][x];
				}
			}
		}
		// Fill the layer with the amount of neurons specified in the topology
		for (int y = 0; y < sight_size; y++)
		{
			for (int x = 0; x < sight_size * 2; x++)
			{
				new_1d_layer.push_back(new_first_layer[y][x]);
			}
		}
		for (size_t i = 0; i < server->GetTakenColors().size(); i++)
		{
			new_1d_layer.push_back(old_2d_first_layer.at(old_size).at(i));
		}
		net[0] = new_1d_layer;
	}
	else if (diff < 0)
	{
		diff = -diff;
		if (diff % 2 == 0)
		{
			int one_for_last_layer = 1;
			old_2d_first_layer.erase(old_2d_first_layer.begin(), old_2d_first_layer.begin() + (diff/2) - 1);
			old_2d_first_layer.erase(old_2d_first_layer.end() - (diff / 2) - one_for_last_layer, old_2d_first_layer.end());
			for (size_t i = 0; i < old_2d_first_layer.size() - 1; i++)
			{
				old_2d_first_layer.at(i).erase(old_2d_first_layer[i].begin(), old_2d_first_layer[i].begin() + (diff / 2) - 1);
				old_2d_first_layer.at(i).erase(old_2d_first_layer[i].end() - (diff / 2) - one_for_last_layer, old_2d_first_layer[i].end());
			}
		}
		else
		{
			int one_for_last_layer = 1;
			old_2d_first_layer.erase(old_2d_first_layer.begin(), old_2d_first_layer.begin() + ((diff + 1) / 2) - 1);
			old_2d_first_layer.erase(old_2d_first_layer.end() - ((diff-1) / 2) - one_for_last_layer, old_2d_first_layer.end() - one_for_last_layer);
			for (size_t i = 0; i < old_2d_first_layer.size() - 1; i++)
			{
				old_2d_first_layer.at(i).erase(old_2d_first_layer[i].begin(), old_2d_first_layer[i].begin() + ((diff+1) / 2) - 1);
				old_2d_first_layer.at(i).erase(old_2d_first_layer[i].end() - ((diff-1) / 2) - one_for_last_layer, old_2d_first_layer[i].end());
			}
		}

		// Fill the layer with the amount of neurons specified in the topology
		for (int y = 0; y < sight_size; y++)
		{
			for (int x = 0; x < sight_size * 2; x++)
			{
				new_1d_layer.push_back(new_first_layer[y][x]);
			}
		}
		for (size_t i = 0; i < server->GetTakenColors().size(); i++)
		{
			new_1d_layer.push_back(old_2d_first_layer.at(sight_size).at(i));
			new_1d_layer.back().my_index = sight_size * sight_size * 2 + i;
		}
		net[0] = new_1d_layer;
	}

}

void NeuralAI::SetInGame(bool ig)
{
	game_won = false;

	AI::SetInGame(ig);
}

void NeuralAI::ClearFitnesses()
{
	fitnesses.clear();
}

void NeuralAI::get_end_tiles()
{
	std::vector<std::vector<tile>> tiles = server->GetBoardState();
	end_tiles.clear();

	int num_visited = 0;
	// Visited array
	uint8_t v[100][100];
	memset(v, 0, sizeof(v));

	std::queue<std::pair<uint8_t, uint8_t>> the_queue;

	k2d::vi2d starting_pos = server->GetStartingPositions()[which_player_am_i];

	the_queue.push({ starting_pos.x, starting_pos.y });
	v[starting_pos.y][starting_pos.x] = 1;

	k2d::vi2d map_size = server->GetMapSize();

	int x = -1;
	int y = -1;
	// run until the queue is empty
	while (!the_queue.empty())
	{
		bool new_found = false;
		// Extrating front pair
		std::pair<uint8_t, uint8_t> coord = the_queue.front();
		x = coord.first;
		y = coord.second;

		num_visited++;

		// Poping front pair of queue
		the_queue.pop();

		// Down
		if (valid_tile(x, y + 1, map_size)
			&& v[y + 1][x] == 0
			&& (tiles[y + 1][x].owner == which_player_am_i))
		{
			the_queue.push({ x, y + 1 });
			v[y + 1][x] = 1;
			new_found = true;
		}
		// Up
		if (valid_tile(x, y - 1, map_size)
			&& v[y - 1][x] == 0
			&& (tiles[y - 1][x].owner == which_player_am_i))
		{
			the_queue.push({ x, y - 1 });
			v[y - 1][x] = 1;
			new_found = true;
		}
		// Right
		if (valid_tile(x + 1, y, map_size)
			&& v[y][x + 1] == 0
			&& (tiles[y][x + 1].owner == which_player_am_i))
		{
			the_queue.push({ x + 1, y });
			v[y][x + 1] = 1;
			new_found = true;
		}
		// Left
		if (valid_tile(x - 1, y, map_size)
			&& v[y][x - 1] == 0
			&& (tiles[y][x - 1].owner == which_player_am_i))
		{
			the_queue.push({ x - 1, y });
			v[y][x - 1] = 1;
			new_found = true;
		}

		if (!new_found)
		{
			end_tiles.push_back({x, y});
		}
	}

}

int NeuralAI::calc_average_fitness()
{
	int sum = 0;
	for (size_t i = 0; i < fitnesses.size(); i++)
	{
		sum += fitnesses.at(i);
	}

	return sum / (int) fitnesses.size();
}

bool NeuralAI::valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size)
{
	if (x < 0 || y < 0) {
		return false;
	}
	if (x >= map_size.x || y >= map_size.y) {
		return false;
	}
	return true;
}