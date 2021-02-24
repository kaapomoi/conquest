#pragma once 

#include <core/Engine.h>
#include <core/GameObject.h>
#include <ui/UIElement.h>
#include <ui/UIGraph.h>
#include <ui/UIFunctionGraph.h>
#include <ui/UIClickableLabel.h>
#include <ui/UIClickableGraph.h>
#include <core/ServerSim.h>
#include <ai/BadAI.h>
#include <ai/SimpleAI.h>
#include <ai/NeuralAI.h>
#include <util/DatabaseHandler.h>
#include <inttypes.h>
#include <time.h>
#include <queue>
#include <random>
#include <tuple>

#define MAKEADDRESS(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)
namespace {

	float pretty_print_function_for_percents(float in)
	{
		float value = (int)(in * 10000 + .5);
		return (float)value / 100;
	}

	float pick_chance_function(float in)
	{
		return 200.0f / (in + 10.0f);
	}

	
}


class ConquestLocal
{
public:
	using Random = effolkronium::random_static;
	ConquestLocal();
	~ConquestLocal();

	int init_engine();

	int load_texture_into_cache(const char* friendly_name, std::string filename);
	k2d::GLTexture load_texture_from_cache(const char* friendly_name);
	k2d::Sprite* create_tile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 5.0f);
	k2d::Sprite* create_projectile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 5.0f);
	k2d::Text* create_text(std::string text, float scale, float depth = 10.f);

	UIElement* get_ui_by_name(std::string name);

	int init_game();
	void InitGeneticAlgorithmValues();

	int create_ui();
	int run();

	int create_ai();
	int PlayGame(AI* first, AI* second);

	void UpdateTileColors();
	void UpdateButtonColors();
	void UpdateUIButtons();
	void UpdateScoreboardColors();
	void UpdateBarColors();
	void UpdateGenerationsText();

	void CalculateNewSelectionWeights();

	void UpdateSelectionWeights();

	void ClampGeneticAlgorithmVariables();

	void CalculateGenerationAverage();
	void CheckIfBestOfGeneration();
	void SetPreviousIdAndTileCount();

	void GetRandomColorFromLoadedSkins(int index);
	int bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x, uint8_t y);

	int main_loop();

	void GeneticAlgorithm();

	void update_input();


	k2d::vi2d WorldToGridPos(const k2d::vi2d world_pos) { return k2d::vi2d((world_pos.x + tile_size.x / 2) / tile_size.x, (world_pos.y + tile_size.y / 2) / tile_size.y); }
	k2d::vi2d GridToWorldPos(const k2d::vi2d grid_pos) { return k2d::vi2d(grid_pos.x * tile_size.x, grid_pos.y * tile_size.y); }

	std::map<GLchar, k2d::Character> LoadFont(const char* _file);

	std::map<GLchar, k2d::Character> LoadChars();

	void read(char* start_position, void* to, int size, int* offset) {
		memcpy(to, start_position + *offset, size);
		*offset += size;
	}

	void write(char* start_position, void* data, int size, int* offset)
	{
		memcpy(start_position + *offset, data, size);
		*offset += size;
	}

	enum class TURN
	{
		PLAYER1 = 0,
		PLAYER2
	};

	typedef struct {
		int packet_type;
		int id;
	} packet_header_t;

	bool valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size)
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
	void HandleEvent(Event &e);

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
	std::vector<std::vector<tile>> tilemap;
	

	// Fonts
	std::map<const char*, std::map<GLchar, k2d::Character>>	font_cache;
	std::map<GLchar, k2d::Character> font1;
	FT_Library		ft;
	FT_Face			face;

	std::vector<UIElement*>			ui_elements;
	std::vector<UIElement*>			bar;
	std::vector<UIClickableLabel*>	 ui_clickable_labels;

	std::string				ini_file_name;
	std::vector<k2d::Color> loaded_skins;
	std::vector<k2d::Color> skins;

	bool scoreboard_init;

	std::vector<player_t> players;

	double timer_counter;
	k2d::vi2d map_size;

	int spectator_id;

	int num_players = 0;

	TURN whose_turn = TURN::PLAYER1;

	uint8_t num_colors;

	std::vector<bool> taken_colors;
	std::vector<std::pair<int, int>> num_of_each_color;

	std::mt19937 random_engine;

	std::vector<k2d::vi2d> sc;

	ServerSim server_sim;

	int turns_played;

	// Vector of AI agents
	std::vector<AI*> ai_agents;

	bool game_in_progress = false;

	// Database
	DatabaseHandler* db_handler;
	const char* db_dir;


	// Genetic algo
	int last_played_index;
	int running_agent_id;

	int epoch;

	int population_size;

	BadAI* bad_ai;
	SimpleAI* simple_ai;

	AI* opponent;

	// Genetic algo variables!!!!
	float top_percentile;

	float mutation_rate;
	float close_mutation_rate;
	double close_mutation_epsilon;

	float mutation_type_chance;

	int		variable_change_multiplier;

	std::vector<float> selection_weights;
	// Is the simulation paused
	bool ui_enabled;

	bool paused;

	bool bad_ai_enabled;

	bool should_create_new_map;

	double average_score_this_generation;

	int current_best_of_gen_id;
	int current_best_of_gen_tiles_owned;

	int previous_id;
	int previous_tiles_owned;

	k2d::vi2d scaled_ui;

	UIGraph* generation_history;
	UIGraph* current_gen_tiles_owned_histogram;
	UIClickableGraph* pick_chance_graph;
};