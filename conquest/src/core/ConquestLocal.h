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
#include <core/ServerSim.h>
#include <ai/BadAI.h>
#include <ai/SimpleAI.h>
#include <ai/NeuralAI.h>
#include <util/DatabaseHandler.h>
#include <json.hpp>
#include <iomanip>
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
}


class ConquestLocal : public k2d::Application
{
public:
	using Random = effolkronium::random_static;
	ConquestLocal(std::string title, int width, int height, int target_fps, bool v_sync);
	~ConquestLocal();

	

	UIBase* get_ui_by_name(std::string name);
	UIButton* get_button_by_name(std::string name);

	float weight_selection_function(float x, float a, float b);
	int init_game();
	void InitGeneticAlgorithmValues();

	int create_ui();

	void Setup() override;

	void PreRender() override;
	void Update() override;


	int create_ai();
	int PlayGame(AI* first, AI* second);

	void UpdateTileColors();
	void UpdateScoreboardIds();


	// Callback functions for buttons
	void CalculateNewSelectionWeights();
	void UpdateSelectionWeights();

	void IncreaseWeightSlope();
	void ResetWeightSlope();
	void DecreaseWeightSlope();

	void PauseGame();
	void ToggleMapCreation();
	void ToggleOpponentType();

	void ClampGeneticAlgorithmVariables();
	void ClampWeightSelectionVariables();
	void ClampTileBrightness();

	void UpdateTileBrightness();

	void SetTargetFpsLow();
	void SetTargetFpsMed();
	void SetTargetFpsHigh();
	void SetTargetFpsUnlimited();

	void UpdateScorebarValues();

	void UpdateProgressBarValues();

	void CalculateGenerationAverage();
	void CheckIfBestOfGeneration();
	void SetPreviousIdAndTileCount();

	void SaveGeneticAlgorithmVariablesToFile(std::string file_name);

	void GetRandomColorFromLoadedSkins(int index);
	int bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x, uint8_t y);

	void GeneticAlgorithm();

	void update_input();

	k2d::vi2d WorldToGridPos(const k2d::vi2d world_pos) { return k2d::vi2d((world_pos.x + tile_size.x / 2) / tile_size.x, (world_pos.y + tile_size.y / 2) / tile_size.y); }
	k2d::vi2d GridToWorldPos(const k2d::vi2d grid_pos) { return k2d::vi2d(grid_pos.x * tile_size.x, grid_pos.y * tile_size.y); }

	k2d::Sprite* create_tile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 25.0f);
	k2d::Sprite* CreateDefaultSprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 25.0f);
	k2d::Sprite* create_projectile_sprite(const char* texture_name, k2d::Color color = k2d::Color(255), float depth = 25.0f);
	k2d::Text* create_text(std::string text, float scale, float depth = 10.f);
	k2d::Text* create_text(std::string text, k2d::vi2d position, float scale, float depth = 10.f);
	

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

	float find_max_local(int first, int last);

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

	UIGraph* generation_history;
	UIGraph* current_gen_tiles_owned_histogram;
	UIScoreBar* scorebar;
	UIClickableGraph* pick_chance_graph;


	std::string				ini_file_name;
	std::vector<k2d::Color> loaded_skins;
	std::vector<k2d::Color> skins;

	bool scoreboard_init;

	std::vector<player_t> players;

	k2d::vi2d map_size;

	int spectator_id;

	int num_players = 0;

	uint8_t num_colors;

	std::vector<bool> taken_colors;
	std::vector<std::pair<int, int>> num_of_each_color;

	std::mt19937 random_engine;

	std::vector<k2d::vi2d> sc;

	ServerSim* server_sim;

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
	float topology_mutation_chance;

	// When topology mutation triggers, randomize if the number of layers also changes
	float num_layers_mutation_chance;


	int		variable_change_multiplier;

	std::vector<float> selection_weights;
	// Is the simulation paused
	bool ui_enabled;

	bool paused;

	bool bad_ai_enabled;

	bool should_create_new_map;

	int half_of_tiles;

	int average_score_this_generation;

	int current_best_of_gen_id;
	int current_best_of_gen_tiles_owned;

	int previous_id;
	int previous_tiles_owned;

	k2d::vi2d scaled_ui;

	int p0_id;
	int p1_id;
	
	int p0_tiles;
	int p1_tiles;

	k2d::Color p0_color;
	k2d::Color p1_color;

	float weight_selection_a;
	float weight_selection_b;
};

