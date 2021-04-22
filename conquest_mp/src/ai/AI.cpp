#include <ai/AI.h>

AI::AI(std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr)
	: which_player_am_i(0), current_turn_players_id(0), taken_colors(0), games_played(0), games_won(0), tiles_owned(0)
{
	this->tilemap = tilemap_ptr;
	this->taken_colors = taken_colors_ptr;
}

AI::~AI()
{
	delete tilemap;
	delete taken_colors;
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
