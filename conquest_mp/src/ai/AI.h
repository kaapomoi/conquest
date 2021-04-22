#pragma once
#include <core/Engine.h>
#include <queue>

// tiles of tilemap 
typedef struct {
	uint8_t color;
	uint8_t owner;
} tile;

class AI
{
public:
	AI(std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr);
	virtual ~AI();

	virtual int Update() = 0;
	
	// SEtters
	virtual void SetCurrentTurnPlayersId(int t_id);

	virtual void SetWhichPlayerAmI(int who);
	virtual void SetTilesOwned(int tiles);

	virtual void AddGamePlayed();
	virtual void AddGameWon();

	// Getters
	virtual int GetGamesPlayed();
	virtual int GetGamesWon();
	virtual int GetTilesOwned();

protected:
	std::vector<std::vector<tile>>* tilemap;
	std::vector<bool>* taken_colors;

	int games_played;
	int games_won;
	int current_turn_players_id;
	int which_player_am_i;
	int tiles_owned;
};