#include <ai/NeuralAI.h>

NeuralAI::NeuralAI(int id, ServerSim* server_sim) :
	AI(id, server_sim), 
	// Set the topology of the net
	neural_net({(server_sim->GetMapSize().x * server_sim->GetMapSize().y) + 3,
		100,
		100,
		100,
	    (int) server_sim->GetTakenColors().size()})
{
	rand_engine.seed(time(NULL));
	tiles_owned = 0;
}

NeuralAI::~NeuralAI()
{

}

void NeuralAI::Update()
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
		std::string encoded_turn_history = tokens[5];
		std::string initial_board_state = data;

		if (winner_id == client_id)
		{
			AddGameWon();
		}

		AddGamePlayed();

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

		break;
	}
	case EventType::INVALID_COLOR:
	{

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
				board_state_single_dimension.push_back((double)board_state[i][j].color);
			}
		}

		// Players owned colors
		board_state_single_dimension.push_back(server->GetPlayers()[0].num_owned);
		board_state_single_dimension.push_back(server->GetPlayers()[1].num_owned);
		board_state_single_dimension.push_back(which_player_am_i);

		// Input the board state into the net
		neural_net.FeedForward(board_state_single_dimension);

		// Get the results from the net
		neural_net.GetResults(result_vec);
		results_map.clear();

		for (size_t i = 0; i < result_vec.size(); i++)
		{
			if (taken_colors.at(i) == false)
			{
				results_map.push_back(std::make_pair(i, result_vec[i]));
			}
		}

		// highest first
		std::sort(results_map.begin(), results_map.end(),
			[](const std::pair<double, int>& a, const std::pair<double, int>& b) -> bool
			{
				return a.first > b.first;
			}
		);

		res = results_map[0].first;

		// Send the number to the server
		SendInputToServer(res);
	}
}

void NeuralAI::SendInputToServer(int input_num)
{
	server->ReceiveInput(client_id, input_num);
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
