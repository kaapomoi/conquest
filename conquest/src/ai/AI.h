#pragma once
#include <core/Engine.h>
#include <core/EventQueue.h>
#include <core/ServerSim.h>

class AI
{
public:
	AI(int id, ServerSim* server_sim);
	virtual ~AI();

	virtual void Update() = 0;

	virtual bool SendInputToServer(int input_num) = 0;

	virtual void GetTakenColorsFromServer() = 0;

	virtual Event GetNextEventFromServer() = 0;
	
	// SEtters
	virtual void SetInGame(bool f);

	virtual void SetClientId(int c_id);

	virtual void SetCurrentTurnPlayersId(int t_id);

	virtual void SetWhichPlayerAmI(int who);
	virtual void SetTilesOwned(int tiles);

	virtual void AddGamePlayed();
	virtual void AddGameWon();

	// Getters
	virtual int GetClientId();
	virtual bool GetInGame();

	virtual int GetGamesPlayed();
	virtual int GetGamesWon();
	virtual int GetTilesOwned();

protected:
	ServerSim* server;
	std::vector<bool> taken_colors;
	
	bool ingame;
	int games_played;
	int games_won;
	int client_id;
	int current_turn_players_id;
	int which_player_am_i;
	int tiles_owned;
};