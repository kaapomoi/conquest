#pragma once
#include <core/Engine.h>
#include <random>
#include <time.h>
#include <algorithm>
#include <queue>
#include <core/EventQueue.h>

const int DEFAULT_NR_COLORS = 5;
const int DEFAULT_WHOSE_TURN = 0;
const int DEFAULT_NUM_TURNS = 0;
const int MAX_PLAYERS = 4;

typedef struct {
	int id;
	uint8_t num_owned;
	int tiles_owned;
} player_t;

typedef struct {
	uint8_t color;
	uint8_t owner;
} tile;

class ServerSim
{
// Public functions
public:
	ServerSim(k2d::vi2d map_size, int num_colors);
	~ServerSim();


	// Connect to this game "server", returns true if it went well.
	bool ConnectToServer(int player_id);

	// Receive input from the "clients", simulates receiving packets
	void ReceiveInput(int player_id, int input_number);

	// Start a game with the players connected to the "server"
	void StartGame();

	// Get the tilemap
	std::vector<std::vector<tile>> GetBoardState();

	std::vector<bool> GetTakenColors();

	Event GetNextEventFromQueue(int client_id);

	std::vector<player_t> GetPlayers();

	std::vector<k2d::vi2d> GetStartingPositions();

	int GetTurnsPlayed();
	bool GetGameInProgress();

	// Update every frame
	void Update();

	k2d::vi2d GetMapSize();

// Private functions 
private:
	player_t new_player(int id);
	
	void create_game(int num_players);

	// Inits the taken colors for the amount of players connected to the server. Also inits color_owned and scores for players.
	void init_players_and_taken_colors();

	// Gets the adjacent tiles that are inside the map bounds
	std::vector<k2d::vi2d> get_adjacent_tiles(k2d::vi2d map_size, k2d::vi2d pos);

	// Checks if the tile is within the map
	bool valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size);

	// Forces a tile to not be a certain color
	void force_change_to(uint8_t x, uint8_t y, uint8_t forbid,
	 std::uniform_int_distribution<int>& num_gen);

	// Gets the adjacent tiles that are inside the map bounds and dont have the same owner as the starting tile
	std::vector<k2d::vi2d> get_adjacent_foreign_tiles(k2d::vi2d map_size, uint8_t owner, k2d::vi2d pos);

	// Changes the color of tiles with specified owner
	void flood_fill_color_change(k2d::vi2d map_size, uint8_t x, uint8_t y, uint8_t owner, uint8_t new_color);

    // Breadth-first-search algorithm that changes the owner of all floodfillable tiles
	int bfs_owner_change(k2d::vi2d map_size, uint8_t x, uint8_t y, uint8_t new_owner, uint8_t color);

	

// Private variables
private:
	// Keep track of whose turn it is to make a move.
	int whose_turn;
	// Keep track num turns played
	int num_turns;
	// The number of colors in the game
	int NR_OF_COLORS;
	// Running id for events
	int running_id;

	bool game_in_progress;

	// Keep track of taken colors
	std::vector<bool> taken_colors;
	// The map / board / grid size
	k2d::vi2d map_size;
	// The players starting positions, [0,0], [mapsize.x - 1, mapsize.y - 1], etc.
	std::vector<k2d::vi2d> s_pos;

	// The board
	std::vector<std::vector<tile>> tilemap;
	// Connected players
	std::vector<player_t> players;
	// Connected players' ids
	std::vector<int> player_ids;

	// Random engine
	std::mt19937 rand_engine;

	// Event queue for "server - client" messaging
	EventQueue event_queue;
};