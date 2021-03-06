#include <ai/AI.h>

AI::AI(int id, ServerSim* server_sim)
	:client_id(id), which_player_am_i(0), current_turn_players_id(0), taken_colors(0), ingame(false), server(server_sim), games_played(0), games_won(0), tiles_owned(0), fitness(0)
{
	
}

AI::~AI()
{

}


// Setters
void AI::SetInGame(bool f)
{
	ingame = f;
}

void AI::SetClientId(int c_id)
{
	client_id = c_id;
}

void AI::SetCurrentTurnPlayersId(int t_id)
{
	current_turn_players_id = t_id;
}

void AI::SetWhichPlayerAmI(int who)
{
	which_player_am_i = who;
}

void AI::SetTilesOwned(int tiles)
{
	tiles_owned = tiles;
}

void AI::AddGamePlayed()
{
	games_played++;
}

void AI::AddGameWon()
{
	games_won++;
}

// Getters
int AI::GetClientId()
{
	return client_id;
}

bool AI::GetInGame()
{
	return ingame;
}

int AI::GetGamesPlayed()
{
	return games_played;
}

int AI::GetGamesWon()
{
	return games_won;
}

int AI::GetTilesOwned()
{
	return tiles_owned;
}
