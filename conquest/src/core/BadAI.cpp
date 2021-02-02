#include <core/BadAI.h>

BadAI::BadAI(int id, ServerSim* server_sim):
	AI(id, server_sim)
{
	rand_engine.seed(time(NULL));
}

BadAI::~BadAI()
{
}

void BadAI::Update()
{
	// Check the server event queue for updates
	Event e = GetNextEventFromServer();

	switch (e.GetType())
	{
	case EventType::GAME_OVER:
	{

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

		// Randomize the input number
		std::uniform_int_distribution<int> num_gen(0, taken_colors.size() - 1);
		
		int res = -1;
		do
		{
			res = num_gen(rand_engine);
		} while (taken_colors.at(res));

		// Send the number to the server
		SendInputToServer(res);
	}


}

void BadAI::SendInputToServer(int input_num)
{
	server->ReceiveInput(client_id, input_num);
}

void BadAI::GetTakenColorsFromServer()
{
	taken_colors = server->GetTakenColors();
}

Event BadAI::GetNextEventFromServer()
{
	return server->GetNextEventFromQueue(client_id);
}
