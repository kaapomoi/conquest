#pragma once 

#include <core/Application.h>
#include <core/GameObject.h>
#include <ui/UIGraph.h>
#include <ui/UIFunctionGraph.h>
#include <ui/UIClickableLabel.h>
#include <ui/UIClickableGraph.h>
#include <ui/UIToggleButton.h>
#include <ui/UIMultiLabel.h>
#include <ui/UIProgressBar.h>
#include <ui/UIScoreBar.h>
#include <ui/UINetDisplay.h>
#include <ui/UIList.h>
#include <ui/UIRectangle.h>
#include <ai/BadAI.h>
#include <ai/SimpleAI.h>
#include <ai/NeuralAI.h>
#include <util/NeuralSaver.h>
#include <json.hpp>
#include <iomanip>
#include <inttypes.h>
#include <time.h>
#include <queue>
#include <random>
#include <tuple>

#define MAX_PLAYERS 8

#define MAKEADDRESS(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

namespace {

	float pretty_print_function_for_percents(float in)
	{
		float value = (int)(in * 10000 + .5);
		return (float)value / 100;
	}
}

enum class GameState {
	NOTCONNECTED = 0,
	INGAME,
};

using Random = effolkronium::random_static;

class ConquestMultiplayer : public k2d::Application
{
public:
	ConquestMultiplayer(std::string title, int width, int height, int target_fps, bool v_sync);
	~ConquestMultiplayer();

	UIBase* get_ui_by_name(std::string name);
	UIButton* get_button_by_name(std::string name);

	float weight_selection_function(float x, float a, float b);
	int init_game();

	int create_ui();

	void Setup() override;

	void PreRender() override;
	void Update() override;
	void CleanUp() override;

	int create_ai();

	int HandleAI();

	void UpdateTileColors();
	void UpdateScoreboardIds();

	void StartGame();

	void ClampTileBrightness();

	
	void UpdateUIElementPositions();
	void UpdateTileBrightness();

	void RevealUI();
	
	void UpdateScorebarValues();

	void ToggleAiType();

	void UpdateDebugRectanglePosition();


	void GetRandomColorFromLoadedSkins(int index);
	int bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x, uint8_t y);

	int SendCommandToServer(uint8_t code, bool& should_do_something);

	void update_input();

	k2d::vi2d WorldToGridPos(const k2d::vi2d world_pos) { return k2d::vi2d((world_pos.x + tile_size.x / 2) / tile_size.x, (world_pos.y + tile_size.y / 2) / tile_size.y); }
	k2d::vi2d GridToWorldPos(const k2d::vi2d grid_pos) { return k2d::vi2d(grid_pos.x * tile_size.x, grid_pos.y * tile_size.y); }

	k2d::Sprite* create_tile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 25.0f);
	k2d::Sprite* CreateDefaultSprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 25.0f);
	k2d::Sprite* create_projectile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 25.0f);
	k2d::Text* create_text(std::string text, float scale, float depth = 10.f);
	k2d::Text* create_text(std::string text, k2d::vi2d position, float scale, float depth = 10.f);
	

	typedef struct {
		int id;
		uint8_t num_owned;
		int tiles_owned;
		struct sockaddr_in address;
	}player_t;

	player_t new_player(int id, int num_owned, struct sockaddr_in* address) {
		player_t player;
		player.id = id;
		player.num_owned = num_owned;
		memcpy(&player.address, address, sizeof(player.address));
		return player;
	}

	void read(char* start_position, void* to, int size, int* offset) {
		memcpy(to, start_position + *offset, size);
		*offset += size;
	}

	void write(char* start_position, void* data, int size, int* offset)
	{
		memcpy(start_position + *offset, data, size);
		*offset += size;
	}

	typedef struct {
		uint8_t x;
		uint8_t y;
	} vc2d;

	

	enum class TURN
	{
		PLAYER1 = 0,
		PLAYER2
	};

	typedef struct {
		int packet_type;
		int id;
	} packet_header_t;

	bool valid_tile(uint8_t x, uint8_t y, vc2d map_size)
	{
		if (x < 0 || y < 0) {
			return false;
		}
		if (x >= map_size.x || y >= map_size.y) {
			return false;
		}
		return true;
	}

private:

	k2d::vi2d tile_size;

	k2d::vi2d mouse_grid_pos;
	bool debug;

	std::vector<GameObject*> tiles;

	// Tile map
	std::vector<std::vector<tile>> tilemap;

	// UI
	float							tile_brightness;

	std::vector<UIButton*>			ui_buttons;
	std::vector<UIMultiLabel*>		ui_multilabels;
	std::vector<UIClickableLabel*>	ui_clickable_labels;
	std::vector<UIProgressBar*>		ui_progressbars;
	std::vector<UIBase*>			all_of_the_ui;

	UIScoreBar* scorebar;
	UIRectangle* nn_vision_rect;
	UINetDisplay* nn_display;

	std::string				ini_file_name;
	std::vector<k2d::Color> loaded_skins;
	std::vector<k2d::Color> skins;

	bool scoreboard_init;

	// Ai agents
	BadAI* bad_ai;
	SimpleAI* simple_ai;
	NeuralAI* neural_ai;

	// Current ai agent in use
	AI* current_ai;

	/// <summary>
	/// The client sends this value to the server
	/// </summary>
	int input_num;

	float send_packets_every_ms;
	double timer_counter;

	vc2d map_size;
	k2d::vi2d scaled_ui;

	int player_id;

	unsigned long NON_BLOCKING = 1;

	uint32_t SERVER_ADDRESS;
	uint16_t SERVER_PORT;

	WSADATA wsa_data;
	int32_t result;

	SOCKET sock;

	uint32_t address;

	struct sockaddr_in socket_address = {};

	struct sockaddr_in server_address = {};

	int32_t address_length;
	struct sockaddr_in sender_address = { 0 };
	char buffer[8192] = { 0 };

	int num_players = 0;
	player_t players[MAX_PLAYERS];

	TURN whose_turn = TURN::PLAYER1;

	uint8_t num_colors;

	std::vector<bool> taken_colors;
	std::vector<std::pair<int, int>> num_of_each_color;
	bool should_send_ready;
	bool should_send_reset;
	bool should_send_drop_all;
	int turn_id;
	int this_player_index;
	std::mt19937 random_engine;
	int header_size;

	std::vector<k2d::vi2d> sc;

	float blink_timer;
	float blink_every_second;

	int winner_index;
	bool game_over;

	int try_to_play_best;

	bool AI_CONTROL;
	bool should_update_ai;
	bool input_pressed;

	// Score bar variables
	int p0_id;
	int p1_id;

	int p0_tiles;
	int p1_tiles;

	k2d::Color p0_color;
	k2d::Color p1_color;
	
	// UI variables
	int		variable_change_multiplier;

	bool	ui_enabled;

	// Game variables
	int turns_played;

	GameState game_state;

};

