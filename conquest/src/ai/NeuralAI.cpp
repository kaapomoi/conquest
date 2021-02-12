#include <ai/NeuralAI.h>

NeuralAI::NeuralAI(int id, ServerSim* server_sim) :
	AI(id, server_sim), 
	// Set the topology of the net
	neural_net({(server_sim->GetMapSize().x * server_sim->GetMapSize().y) + 2,
		100,
		100,
		100,
	    (int) server_sim->GetTakenColors().size()})
{
	rand_engine.seed(time(NULL));
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
		std::string encoded_turn_history = tokens[3];
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
		SetCurrentTurnPlayersId(std::stoi(data));
		

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
		//neural_net.FeedForward();



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
