#pragma once 

#include <core/Engine.h>
#include <core/GameObject.h>
#include <ui/UIElement.h>
#include <core/ServerSim.h>
#include <inttypes.h>
#include <time.h>
#include <queue>
#include <random>
#include <tuple>

#define MAKEADDRESS(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

class Conquest
{
public:
	Conquest();
	~Conquest();

	int init_engine();

	int load_texture_into_cache(const char* friendly_name, std::string filename);
	k2d::GLTexture load_texture_from_cache(const char* friendly_name);
	k2d::Sprite* create_tile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 5.0f);
	k2d::Sprite* create_projectile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 5.0f);
	k2d::Text* create_text(std::string text, float scale, float depth = 10.f);

	UIElement* get_ui_by_name(std::string name);

	int init_game();

	int create_objects();
	int create_ui();
	int create_ui_unit_card();
	int run();
	void UpdateTileColors();
	void UpdateButtonColors();
	void UpdateScoreboardColors();
	void UpdateBarColors();
	
	void GetRandomColorFromLoadedSkins(int index);
	int bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x, uint8_t y);
	void HandleAI();

	int SendCommandToServer(uint8_t code, bool& should_do_something);

	int main_loop();
	void update_input();


	k2d::vi2d WorldToGridPos(const k2d::vi2d world_pos) { return k2d::vi2d((world_pos.x + tile_size.x / 2) / tile_size.x, (world_pos.y + tile_size.y / 2) / tile_size.y); }
	k2d::vi2d GridToWorldPos(const k2d::vi2d grid_pos) { return k2d::vi2d(grid_pos.x * tile_size.x, grid_pos.y * tile_size.y); }

	std::map<GLchar, k2d::Character> LoadFont(const char* _file);

	std::map<GLchar, k2d::Character> LoadChars();

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

	// tiles of tilemap 
	typedef struct {
		uint8_t color;
		uint8_t owner;
	} tile;

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
	// Engine variables
	k2d::Engine* engine;
	k2d::SpriteBatch* sprite_batch;

	std::string window_title;
	int window_width;
	int window_height;
	bool v_sync;

	float fps_target;
	double dt;

	float camera_mvmt_speed;

	// Game variables
	std::map<const char*, k2d::GLTexture> m_texture_cache;

	k2d::vi2d world_size;
	k2d::vi2d tile_size;
	k2d::vi2d origin;

	k2d::vi2d mouse_grid_pos;
	bool debug;

	std::vector<GameObject*> tiles;

	// Tile map
	tile** tilemap;

	// Helpers for pathfinding
	struct smaller {
		bool operator()(const k2d::vi2d& lhs, const k2d::vi2d rhs) const
		{
			return std::tie(lhs.x, lhs.y) < std::tie(rhs.x, rhs.y);
		}
	}; 
	

	// Fonts
	std::map<const char*, std::map<GLchar, k2d::Character>>	font_cache;
	std::map<GLchar, k2d::Character> font1;
	FT_Library		ft;
	FT_Face			face;

	std::vector<UIElement*> ui_elements;
	std::vector<UIElement*> buttons;
	std::vector<UIElement*> bar;
	UIElement*				turns_text;


	std::string				ini_file_name;
	std::vector<k2d::Color> loaded_skins;
	std::vector<k2d::Color> skins;

	bool scoreboard_init;

	/// <summary>
	/// The client sends this value to the server
	/// </summary>
	int input_num;

	float send_packets_every_ms;
	double timer_counter;
	vc2d map_size;

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
	player_t players[12];

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
};