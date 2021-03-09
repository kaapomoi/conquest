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
	this->sight_size = parent.sight_size;
	this->parent_ids = parent.parent_ids;
	this->parent_ids.push_back(parent.client_id);
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
			vision_grid_position = find_furthest_owned_tile();
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

					uint8_t owner = board_state[i][j].owner;
					if (owner == -9)
					{
						// this tile is neutral
						board_state_single_dimension.push_back(0);
					}
					else if (owner != which_player_am_i)
					{
						// Opponent owns this tile
						board_state_single_dimension.push_back(-1);
					}
					else
					{
						// We own this tile
						board_state_single_dimension.push_back(1);
					}



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
				net[i].insert(net[i].begin() + j, Neuron(net[i + 1].size(), j));

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

void NeuralAI::SetInGame(bool ig)
{
	game_won = false;

	AI::SetInGame(ig);
}

k2d::vi2d NeuralAI::find_furthest_owned_tile()
{
	std::vector<std::vector<tile>> tiles = server->GetBoardState();

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
		}
		// Up
		if (valid_tile(x, y - 1, map_size)
			&& v[y - 1][x] == 0
			&& (tiles[y - 1][x].owner == which_player_am_i))
		{
			the_queue.push({ x, y - 1 });
			v[y - 1][x] = 1;
		}
		// Right
		if (valid_tile(x + 1, y, map_size)
			&& v[y][x + 1] == 0
			&& (tiles[y][x + 1].owner == which_player_am_i))
		{
			the_queue.push({ x + 1, y });
			v[y][x + 1] = 1;
		}
		// Left
		if (valid_tile(x - 1, y, map_size)
			&& v[y][x - 1] == 0
			&& (tiles[y][x - 1].owner == which_player_am_i))
		{
			the_queue.push({ x - 1, y });
			v[y][x - 1] = 1;
		}
	}

	k2d::vi2d last_visited_tile(x, y);

	return last_visited_tile;

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