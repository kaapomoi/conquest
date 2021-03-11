#include <core/ConquestLocal.h>

ConquestLocal::ConquestLocal(std::string title, int width, int height, int target_fps, bool v_sync) :
	k2d::Application(title, width, height, target_fps, v_sync),
	server_sim(nullptr)
{
	Setup();
}

ConquestLocal::~ConquestLocal()
{
}

void ConquestLocal::Setup()
{
	// Basics
	SetShaders("Shaders/core.vert", "Shaders/core.frag", "core", { "vertex_position", "vertex_color", "vertex_uv" });
	engine->SetCameraPosition(k2d::vi2d(110, 230));
	font1 = LoadFont("Fonts/opensans.ttf");

	spectator_id = -9999;

	// Session id for the database table. time from 1-1-2000
	time_t t;
	struct tm y2k = { 0 };
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
	time(&t);
	int ses_id = difftime(t, mktime(&y2k));

	// Database init
	db_dir = "Data/TEST.db";
	db_handler = new DatabaseHandler(ses_id, db_dir);
	db_handler->CreateMatchesTable();

	// Selection weights init
	weight_selection_a = 200.0f;
	weight_selection_b = 10.0f;

	num_maps = 2;

	// Init game
	InitGeneticAlgorithmValues();

	init_game();
	create_ai();
	//
	create_ui();
	//create_ui_unit_card();

	CalculateNewSelectionWeights();
	UpdateSelectionWeights();

	half_of_tiles = server_sim->GetMapSize().x * server_sim->GetMapSize().y * 0.5;

	// Start the main loop by calling base::Setup()
	k2d::Application::Setup();
}

float ConquestLocal::weight_selection_function(float x, float a, float b)
{
	return a / (x + b);
}

int ConquestLocal::init_game()
{
	// Init with mapsize, colors
	server_sim = new ServerSim(k2d::vi2d(40, 30), 6, num_maps);
	load_texture_into_cache("empty", "Textures/tiles/square100x100.png");
	load_texture_into_cache("selected", "Textures/tiles/selection100x100.png");
	load_texture_into_cache("ss", "Textures/tiles/ss100x100.png");
	load_texture_into_cache("dot", "Textures/tiles/dot100x100.png");
	load_texture_into_cache("full", "Textures/tiles/full100x100.png");
	load_texture_into_cache("half", "Textures/tiles/halfalpha100x100.png");


	load_texture_into_cache("ui", "Textures/ui/ui.png");
	load_texture_into_cache("unitcard", "Textures/ui/ui100x300.png");

	tile_size.x = 20;
	tile_size.y = 20;

	std::ifstream videofile("config/video.txt");
	if (!videofile) {
		k2d::KUSI_ERROR("VIDEO CONFIG LOADING ERROR Q");
	}
	int p;
	videofile >> p;

	tile_size.x = p;
	tile_size.y = p;

	videofile.close();

	std::ifstream myfile("config/skins.txt");
	if (!myfile) {
		k2d::KUSI_ERROR("SKIN LOADING ERROR");
	}
	int r;
	int g;
	int b;
	while (myfile >> r >> g >> b)
	{
		loaded_skins.push_back(k2d::Color(r,g,b, 200));
	}
	myfile.close();

	for (size_t i = 0; i < 20; i++)
	{
		skins.push_back(loaded_skins.at(i));
	}

	// TODO set this somehow
	map_size = server_sim->GetMapSize();
	num_colors = server_sim->GetTakenColors().size();

	// Randomizer init
	random_engine.seed((unsigned int) time(NULL));

	return 0;
}

void ConquestLocal::InitGeneticAlgorithmValues()
{
	//// 0.1f = 10%;
	/*top_percentile = 0.2f;

	mutation_rate = 0.01f;
	close_mutation_rate = 0.05f;
	close_mutation_epsilon = 0.50;

	topology_mutation_chance = 0.01f;
	num_layers_mutation_chance = 0.1f;

	mutation_type_chance = 0.95f;
	variable_change_multiplier = 1;*/
	
	// Load from file

	std::string filename = "Data/Gen.json";

	std::ifstream i(filename);
	nlohmann::json j;
	i >> j;

	close_mutation_epsilon = j.find("close_mutation_epsilon").value();
	close_mutation_rate = j.find("close_mutation_rate").value();
	mutation_rate = j.find("mutation_rate").value();
	mutation_type_chance = j.find("mutation_type_chance").value();
	num_layers_mutation_chance = j.find("num_layers_mutation_chance").value();
	population_size = j.find("population_size").value();
	top_percentile = j.find("top_percentile").value();
	topology_mutation_chance = j.find("topology_mutation_chance").value();
	topology_mutation_rate = j.find("topology_mutation_rate").value();
	playstyle_mutation_rate = j.find("playstyle_mutation_rate").value();
	playstyle_mutation_epsilon = j.find("playstyle_mutation_epsilon").value();
	sight_size_mutation_rate = j.find("sight_size_mutation_rate").value();
	sight_size_mutation_epsilon = j.find("sight_size_mutation_epsilon").value();
	num_maps = j.find("num_maps").value();
}

int ConquestLocal::create_ai()
{
	bad_ai = new BadAI(0, server_sim);
	simple_ai = new SimpleAI(1, server_sim);

	opponent = bad_ai;
	bad_ai_enabled = true;

	running_agent_id = 2;
	last_played_index = 0;

	/*std::vector<int> default_topology{ 
		(server_sim->GetMapSize().x * server_sim->GetMapSize().y) + (int)server_sim->GetTakenColors().size(),
		90,
		90,
		(int)server_sim->GetTakenColors().size()};*/

	std::uniform_real_distribution<float> n_d(0.1f, 1.1f);
	std::normal_distribution<> s_d(5, 2);
	int prev_layer_neurons = 0;
	for (size_t i = 0; i < population_size; i++)
	{
		int sight_size = std::round(s_d(random_engine)) + 3;
		prev_layer_neurons = sight_size * sight_size * 2 + (int)server_sim->GetTakenColors().size();

		// Default number of input nodes
		std::vector<int> topology{ sight_size * sight_size * 2 + (int)server_sim->GetTakenColors().size() };


		int num_layers = Random::get(1, 3);
		for (size_t i = 0; i < num_layers; i++)
		{
			prev_layer_neurons = prev_layer_neurons * n_d(random_engine);
			topology.push_back(std::round(prev_layer_neurons) + 5);
		}

		// Default amount of output nodes
		topology.push_back((int) server_sim->GetTakenColors().size());

		ai_agents.push_back(new NeuralAI(running_agent_id++, server_sim, sight_size, topology));
	}

	return 0;
}

int ConquestLocal::create_ui()
{
	scaled_ui = tile_size * 4;
	tile_brightness = 0.5f;

	variable_change_multiplier = 1;

	ui_enabled = true;

#pragma region MultiLabels
	/*
		MULTILINE LABELS
	*/
	// Generation ids
	UIMultiLabel* generation_ids = new UIMultiLabel( "GenerationIds",
		k2d::vi2d(0 - scaled_ui.x * 2, tile_size.y * map_size.y - scaled_ui.y * 0.5f - tile_size.y * 0.5f),
		k2d::vi2d(scaled_ui.x * 3 + tile_size.x, scaled_ui.y * 3),
		tile_size.y,
		0.15f,
		30.0f,
		font1,
		load_texture_from_cache("full"), 
		sprite_batch);
	generation_ids->AddBackground(k2d::Color(255));
	generation_ids->AddLabel("Generation", "Generation: ", &epoch);
	generation_ids->AddLabel("PreviousID", "Previousnn: ", &previous_id);
	generation_ids->AddLabel("GenerationBestID", "Best of gen: ", &current_best_of_gen_id);
	ui_multilabels.push_back(generation_ids);

	// Generation Fitness values
	UIMultiLabel* generation_fitness = new UIMultiLabel("GenerationFs",
		k2d::vi2d(0 - scaled_ui.x * 0.75f, tile_size.y * map_size.y - scaled_ui.y * 0.5f - tile_size.y * 0.5f),
		k2d::vi2d(scaled_ui.x, scaled_ui.y * 3),
		tile_size.y,
		0.15f,
		30.0f,
		font1,
		load_texture_from_cache("full"),
		sprite_batch);
	generation_fitness->AddLabel("GenerationFitness",		"F: ", &average_score_this_generation);
	generation_fitness->AddLabel("PreviousFitness",			"F: ", &previous_tiles_owned);
	generation_fitness->AddLabel("GenerationBestFitness",	"F: ", &current_best_of_gen_tiles_owned);

	ui_multilabels.push_back(generation_fitness);

	// Generation Fitness values
	UIMultiLabel* p0_scoreboard = new UIMultiLabel("P0Scoreboard",
		k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 - tile_size.y / 2),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		tile_size.y,
		0.15f,
		30.0f,
		font1,
		load_texture_from_cache("full"),
		sprite_batch);
	p0_scoreboard->AddBackground(skins.at(0));
	p0_scoreboard->AddLabel("P0Scoreboard", "ID: ", &p0_id);

	ui_multilabels.push_back(p0_scoreboard);

	// Generation Fitness values
	UIMultiLabel* p1_scoreboard = new UIMultiLabel("P1Scoreboard",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y / 2 - tile_size.y / 2),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		tile_size.y,
		0.15f,
		30.0f,
		font1,
		load_texture_from_cache("full"),
		sprite_batch);
	p1_scoreboard->AddBackground(skins.at(1));
	p1_scoreboard->AddLabel("P1Scoreboard", "ID: ", &p1_id);

	ui_multilabels.push_back(p1_scoreboard);


#pragma endregion Multilabels

#pragma region ClickableLabels


	/*
		CLICKABLE LABELS
	*/
	// Tile brightness label
	UIClickableLabel* tile_brightness_label = new UIClickableLabel("TileBrightnessLabel", "A: ",
		k2d::vi2d(0 + map_size.x * tile_size.x + scaled_ui.x * 0.5f + tile_size.x * 0.5f, map_size.y * tile_size.y + tile_size.y * 3.0f),
		k2d::vi2d(-scaled_ui.x * 0.25f, tile_size.y * 0.f - 5),
		k2d::vi2d(scaled_ui.x, scaled_ui.y * 0.25f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.10f, 26.0f, k2d::Color(255));
	tile_brightness_label->SetBackground(k2d::Color(129, 255));
	tile_brightness_label->SetVariable(&tile_brightness);
	tile_brightness_label->SetModifiable(true);
	tile_brightness_label->SetBaseMultiplier(0.05f);
	tile_brightness_label->SetPrintPrecision(2);
	tile_brightness_label->AddCallbackFunction(this, &ConquestLocal::ClampTileBrightness);
	tile_brightness_label->AddCallbackFunction(this, &ConquestLocal::UpdateTileBrightness);

	ui_clickable_labels.push_back(tile_brightness_label);

	// multiplier label
	UIClickableLabel* multiplier = new UIClickableLabel("VariableChangeMultiplier", "Mult.: ",
		k2d::vi2d(0 - scaled_ui.x * 2 - tile_size.x * 2.5f, tile_size.y * 0.5f +scaled_ui.y * 0.5f + 1),
		k2d::vi2d(-scaled_ui.x * 0.80f, tile_size.y * 0.f-5),
		k2d::vi2d(scaled_ui.x * 2, scaled_ui.y * 0.5f -2),
		load_texture_from_cache("full"),
		sprite_batch, font1,
		0.15f, 26.0f, k2d::Color(255));
	multiplier->SetBackground(k2d::Color(129, 255));
	multiplier->SetVariable(&variable_change_multiplier);

	ui_clickable_labels.push_back(multiplier);


	// population size label
	UIClickableLabel* pop_size_label = new UIClickableLabel("PopulationSizeClickable", "Pop.: ",
		k2d::vi2d(0 - scaled_ui.x * 2 - tile_size.x * 2.5f,  tile_size.y * 0.5f + 1),
		k2d::vi2d(-scaled_ui.x * 0.5f, tile_size.y * 0.f-5),
		k2d::vi2d(scaled_ui.x * 2, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.15f, 26.0f, k2d::Color(255));
	pop_size_label->SetBackground(k2d::Color(129, 255));
	pop_size_label->SetVariable(&population_size);
	pop_size_label->SetModifiable(true);
	pop_size_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);
	pop_size_label->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);

	ui_clickable_labels.push_back(pop_size_label);


	// population size label
	UIClickableLabel* num_maps_label = new UIClickableLabel("NumMapsClickable", "Maps: ",
		k2d::vi2d(0 - scaled_ui.x * 2 - tile_size.x * 2.5f, scaled_ui.y + tile_size.x * 1.5f + 1),
		k2d::vi2d(-scaled_ui.x * 0.5f, tile_size.y * 0.f - 5),
		k2d::vi2d(scaled_ui.x * 2, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.15f, 26.0f, k2d::Color(255));
	num_maps_label->SetBackground(k2d::Color(129, 255));
	num_maps_label->SetVariable(&num_maps);
	num_maps_label->SetModifiable(true);
	num_maps_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);
	num_maps_label->AddCallbackFunction(this, &ConquestLocal::CreateNewMaps);
	
	ui_clickable_labels.push_back(num_maps_label);

	/// The grid:
	/// x = 3 2 1
	/// -scaled_ui.x * grid_x * 2.5 + tile_size.x * grid_x * 1 + 1.5


	

	// top percentile label
	UIClickableLabel* top_percentile_label = new UIClickableLabel("TopPercentileClickable", "Top %: ",
		k2d::vi2d(0 - scaled_ui.x * 7.5 + tile_size.x * 1.5, -scaled_ui.y * 0.5f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	top_percentile_label->SetBackground(k2d::Color(129, 255));
	top_percentile_label->SetVariable(&top_percentile);
	top_percentile_label->SetModifiable(true);
	top_percentile_label->SetPrettyPrintFunc(pretty_print_function_for_percents);
	top_percentile_label->SetPrintPrecision(0);
	top_percentile_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);
	top_percentile_label->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);

	ui_clickable_labels.push_back(top_percentile_label);


	// Mutation rate label
	UIClickableLabel* mutation_label = new UIClickableLabel("MutationRateClickable", "R M. Rate: ",
		k2d::vi2d(0 - scaled_ui.x * 7.5  + tile_size.x * 1.5, - scaled_ui.y * 1.0f - tile_size.y * 1+1),
		k2d::vi2d(-scaled_ui.x * 0.7f, tile_size.y * 0.f - 5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1, 
		0.12f, 26.0f, k2d::Color(255));
	mutation_label->SetBackground(k2d::Color(129, 255));
	mutation_label->SetVariable(&mutation_rate);
	//mutation_label->SetPrettyPrintFunc(pretty_print_function_for_percents);
	mutation_label->SetPrintPrecision(6);
	mutation_label->SetBaseMultiplier(0.000001f);
	mutation_label->SetModifiable(true);

	ui_clickable_labels.push_back(mutation_label);

	// Close Mutation rate label
	UIClickableLabel* close_mutation_rate_label = new UIClickableLabel("CloseMutationRateClickable", "C M. Rate: ",
		k2d::vi2d(0 - scaled_ui.x * 7.5 + tile_size.x * 1.5, -scaled_ui.y * 1.5f - tile_size.y * 1+1),
		k2d::vi2d(-scaled_ui.x * 0.7f, - 5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	close_mutation_rate_label->SetBackground(k2d::Color(129, 255));
	close_mutation_rate_label->SetVariable(&close_mutation_rate);
	//mutation_label->SetPrettyPrintFunc(pretty_print_function_for_percents);
	close_mutation_rate_label->SetPrintPrecision(6);
	close_mutation_rate_label->SetBaseMultiplier(0.000001f);
	close_mutation_rate_label->SetModifiable(true);

	ui_clickable_labels.push_back(close_mutation_rate_label);
	


	// Mutation Type Chance rate label
	UIClickableLabel* mutation_type_chance_label = new UIClickableLabel("MutationTypeChanceClickable", "M. Type C: ",
		k2d::vi2d(0 - scaled_ui.x * 7.5 + tile_size.x * 1.5, -scaled_ui.y * 2.0f - tile_size.y * 1+1),
		k2d::vi2d(-scaled_ui.x * 0.7f, - 5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	mutation_type_chance_label->SetBackground(k2d::Color(129, 255));
	mutation_type_chance_label->SetVariable(&mutation_type_chance);
	//mutation_label->SetPrettyPrintFunc(pretty_print_function_for_percents);
	mutation_type_chance_label->SetPrintPrecision(4);
	mutation_type_chance_label->SetBaseMultiplier(0.001f);
	mutation_type_chance_label->SetModifiable(true);

	ui_clickable_labels.push_back(mutation_type_chance_label);

	// topology mutation chance label
	UIClickableLabel* topology_mutation_rate_label = new UIClickableLabel("TopologyMutationRateClickable", "T M.R: ",
		k2d::vi2d(0 - scaled_ui.x * 5.0 + tile_size.x * 2.5, -scaled_ui.y * 2.0f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	topology_mutation_rate_label->SetBackground(k2d::Color(129, 255));
	topology_mutation_rate_label->SetVariable(&topology_mutation_rate);
	topology_mutation_rate_label->SetModifiable(true);
	topology_mutation_rate_label->SetBaseMultiplier(0.0001f);
	topology_mutation_rate_label->SetPrintPrecision(4);
	topology_mutation_rate_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);

	ui_clickable_labels.push_back(topology_mutation_rate_label);

	// num layers mutation chance label
	UIClickableLabel* topology_mutation_chance_label = new UIClickableLabel("TopologyMutationChanceClickable", "T M.%: ",
		k2d::vi2d(0 - scaled_ui.x * 2.5 + tile_size.x * 3.5, -scaled_ui.y * 2.0f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	topology_mutation_chance_label->SetBackground(k2d::Color(129, 255));
	topology_mutation_chance_label->SetVariable(&topology_mutation_chance);
	topology_mutation_chance_label->SetModifiable(true);
	topology_mutation_chance_label->SetBaseMultiplier(0.0001f);
	topology_mutation_chance_label->SetPrintPrecision(4);
	topology_mutation_chance_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);

	ui_clickable_labels.push_back(topology_mutation_chance_label);

	// playstyle mutatiom
	UIClickableLabel* playstyle_mutation_label = new UIClickableLabel("PlaystyleMutationRateClickable", "P M.%: ",
		k2d::vi2d(0 - scaled_ui.x * 2.5 + tile_size.x * 3.5, -scaled_ui.y * 1.5f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	playstyle_mutation_label->SetBackground(k2d::Color(129, 255));
	playstyle_mutation_label->SetVariable(&playstyle_mutation_rate);
	playstyle_mutation_label->SetModifiable(true);
	playstyle_mutation_label->SetBaseMultiplier(0.0001f);
	playstyle_mutation_label->SetPrintPrecision(4);
	playstyle_mutation_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);

	ui_clickable_labels.push_back(playstyle_mutation_label);

	// num layers mutation chance label
	UIClickableLabel* playstyle_mutation_epsilon_label = new UIClickableLabel("PlaystyleMutationEpsilonClickable", "P M.E: ",
		k2d::vi2d(0 - scaled_ui.x * 2.5 + tile_size.x * 3.5, -scaled_ui.y * 1.0f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	playstyle_mutation_epsilon_label->SetBackground(k2d::Color(129, 255));
	playstyle_mutation_epsilon_label->SetVariable(&playstyle_mutation_epsilon);
	playstyle_mutation_epsilon_label->SetModifiable(true);
	playstyle_mutation_epsilon_label->SetBaseMultiplier(0.0001f);
	playstyle_mutation_epsilon_label->SetPrintPrecision(4);
	playstyle_mutation_epsilon_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);

	ui_clickable_labels.push_back(playstyle_mutation_epsilon_label);

	// num layers mutation chance label
	UIClickableLabel* sight_size_mutation_rate_label = new UIClickableLabel("SightSizeMutationRateClickable", "S M.%: ",
		k2d::vi2d(0 - scaled_ui.x * 2.5 + tile_size.x * 3.5, -scaled_ui.y * 0.5f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	sight_size_mutation_rate_label->SetBackground(k2d::Color(129, 255));
	sight_size_mutation_rate_label->SetVariable(&sight_size_mutation_rate);
	sight_size_mutation_rate_label->SetModifiable(true);
	sight_size_mutation_rate_label->SetBaseMultiplier(0.0001f);
	sight_size_mutation_rate_label->SetPrintPrecision(4);
	sight_size_mutation_rate_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);

	ui_clickable_labels.push_back(sight_size_mutation_rate_label);

	// num layers mutation chance label
	UIClickableLabel* sight_size_mutation_epsilon_label = new UIClickableLabel("SightSizeMutationEpsilonClickable", "S M.E: ",
		k2d::vi2d(0 - scaled_ui.x * 5.0 + tile_size.x * 2.5, -scaled_ui.y * 0.5f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.7f, -5),
		k2d::vi2d(scaled_ui.x * 2.5, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	sight_size_mutation_epsilon_label->SetBackground(k2d::Color(129, 255));
	sight_size_mutation_epsilon_label->SetVariable(&sight_size_mutation_epsilon);
	sight_size_mutation_epsilon_label->SetModifiable(true);
	sight_size_mutation_epsilon_label->SetBaseMultiplier(1);
	sight_size_mutation_epsilon_label->SetPrintPrecision(0);
	sight_size_mutation_epsilon_label->AddCallbackFunction(this, &ConquestLocal::ClampGeneticAlgorithmVariables);

	ui_clickable_labels.push_back(sight_size_mutation_epsilon_label);

	// Turns played text
	UIClickableLabel* turns_text = new UIClickableLabel("NRTurnsLabel", "Turns played: ",
		k2d::vi2d(0, tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		k2d::vf2d(0, -tile_size.y * 0.2f),
		k2d::vi2d(0, 0),
		load_texture_from_cache("empty"), sprite_batch, font1,
		0.15f, 35.0f, k2d::Color(255));
	turns_text->SetIsActive(true);
	turns_text->SetModifiable(false);
	turns_text->SetVariable(&turns_played);

	ui_clickable_labels.push_back(turns_text);


#pragma endregion Clickable labels


#pragma region Buttons
	/*
		BUTTONS
	*/
	// Opponent toggle button
	UIToggleButton* opponent_choice = new UIToggleButton("OpponentChoice",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2),
		k2d::vi2d(scaled_ui.x, scaled_ui.y * 0.5f),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("  Bad   Simple", 0.10f, 25.0f),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f),
			scaled_ui.x * 0.5f, scaled_ui.y * 0.5f,
			26.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0, 128), load_texture_from_cache("full"), sprite_batch));
	opponent_choice->SetActive(true);
	opponent_choice->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, -4));
	opponent_choice->AddCallbackFunction(opponent_choice, &UIToggleButton::ToggleFuncSideways);
	opponent_choice->AddCallbackFunction(this, &ConquestLocal::ToggleOpponentType);
	// Ugly position init
	opponent_choice->GetDarkoutSprite()->SetPosition(glm::vec2(opponent_choice->GetDarkoutSprite()->GetPosition().x + opponent_choice->GetSize().x / 4, opponent_choice->GetDarkoutSprite()->GetPosition().y));
	opponent_choice->SetDarkoutActive(true);

	ui_buttons.push_back(opponent_choice);

	UIToggleButton* new_map_button = new UIToggleButton("NewMapButton",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y - tile_size.y * 2.5f),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("New Map", 0.13f, 25.0f),
		CreateDefaultSprite("full", k2d::Color(0, 128), 26.0f));
	new_map_button->SetActive(true);
	new_map_button->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, 0));
	new_map_button->AddCallbackFunction(new_map_button, &UIToggleButton::ToggleFuncOnOff);
	new_map_button->AddCallbackFunction(this, &ConquestLocal::ToggleMapCreation);
	new_map_button->SetDarkoutActive(false);
	ui_buttons.push_back(new_map_button);

	// Pause button
	UIToggleButton* pause_button = new UIToggleButton("PauseButton",
		k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 + scaled_ui.y + tile_size.y * 0.5f),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Pause", 0.15f, 25.0f),
		CreateDefaultSprite("full", k2d::Color(0, 128), 26.0f));
	pause_button->SetActive(true);
	pause_button->SetTextOffset(k2d::vf2d(-tile_size.x * 1.3f, -tile_size.y * 0.2f));
	pause_button->AddCallbackFunction(pause_button, &UIToggleButton::ToggleFuncOnOff);
	pause_button->AddCallbackFunction(this, &ConquestLocal::PauseGame);
	pause_button->SetDarkoutActive(false);

	ui_buttons.push_back(pause_button);


	// Weight selection buttons
	UIButton* decrease_slope_button = new UIButton("DecreaseWeightSlopeButton",
		k2d::vi2d(0 - scaled_ui.x * 0.5f - tile_size.x * 0.5f, tile_size.y * map_size.y * 1.0f - scaled_ui.y * 3.0f + tile_size.y * 1.5f),
		k2d::vi2d(scaled_ui.x * 0.5f, scaled_ui.y * 0.5f),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Up", 0.10f, 25.0f));
	decrease_slope_button->SetActive(true);
	decrease_slope_button->SetTextOffset(k2d::vf2d(-tile_size.x * 0.5f, -tile_size.y * 0.2f));
	decrease_slope_button->AddCallbackFunction(this, &ConquestLocal::DecreaseWeightSlope);
	decrease_slope_button->AddCallbackFunction(this, &ConquestLocal::CalculateNewSelectionWeights);
	decrease_slope_button->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);
	ui_buttons.push_back(decrease_slope_button);


	UIButton* reset_slope_button = new UIButton("ResetWeightSlopeButton",
		k2d::vi2d(0 - scaled_ui.x * 0.5f - tile_size.x * 0.5f, tile_size.y * map_size.y * 1.0f - scaled_ui.y * 4.0f + tile_size.y * 2.5f),
		k2d::vi2d(scaled_ui.x * 0.5f, scaled_ui.y * 0.5f),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Reset", 0.10f, 25.0f));
	reset_slope_button->SetActive(true);
	reset_slope_button->SetTextOffset(k2d::vf2d(-tile_size.x * 0.9f, -tile_size.y * 0.2f));
	reset_slope_button->AddCallbackFunction(this, &ConquestLocal::ResetWeightSlope);
	reset_slope_button->AddCallbackFunction(this, &ConquestLocal::CalculateNewSelectionWeights);
	reset_slope_button->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);
	ui_buttons.push_back(reset_slope_button);


	UIButton* increase_slope_button = new UIButton("IncreaseWeightSlopeButton",
		k2d::vi2d(0 - scaled_ui.x * 0.5f - tile_size.x * 0.5f, tile_size.y * map_size.y * 1.0f - scaled_ui.y * 5.0f + tile_size.y * 3.5f),
		k2d::vi2d(scaled_ui.x * 0.5f, scaled_ui.y * 0.5f),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Down", 0.10f, 25.0f));
	increase_slope_button->SetActive(true);
	increase_slope_button->SetTextOffset(k2d::vf2d(-tile_size.x * 0.9f, -tile_size.y * 0.2f));
	increase_slope_button->AddCallbackFunction(this, &ConquestLocal::IncreaseWeightSlope);
	increase_slope_button->AddCallbackFunction(this, &ConquestLocal::CalculateNewSelectionWeights);
	increase_slope_button->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);
	ui_buttons.push_back(increase_slope_button);

	
	UIButton* fps_button_low = new UIButton("FPSButtonLow",
		k2d::vi2d(0 + map_size.x * tile_size.x + tile_size.x, map_size.y * tile_size.y + tile_size.y * 2.0f),
		k2d::vi2d(scaled_ui.x * 0.25f-2, scaled_ui.y * 0.25f - 2),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Lo", 0.10f, 25.0f));
	fps_button_low->SetActive(true);
	fps_button_low->SetTextOffset(k2d::vf2d(-tile_size.x * 0.45f, -tile_size.y * 0.2f));
	fps_button_low->AddCallbackFunction(this, &ConquestLocal::SetTargetFpsLow);
	ui_buttons.push_back(fps_button_low);

	UIButton* fps_button_med = new UIButton("FPSButtonMed",
		k2d::vi2d(0 + map_size.x * tile_size.x + tile_size.x * 2.0f, map_size.y * tile_size.y + tile_size.y * 2.0f),
		k2d::vi2d(scaled_ui.x * 0.25f-2, scaled_ui.y * 0.25f - 2),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Me", 0.10f, 25.0f));
	fps_button_med->SetActive(true);
	fps_button_med->SetTextOffset(k2d::vf2d(-tile_size.x * 0.45f, -tile_size.y * 0.2f));
	fps_button_med->AddCallbackFunction(this, &ConquestLocal::SetTargetFpsMed);
	ui_buttons.push_back(fps_button_med);

	UIButton* fps_button_high = new UIButton("FPSButtonHigh",
		k2d::vi2d(0 + map_size.x * tile_size.x + tile_size.x * 3.0f, map_size.y * tile_size.y + tile_size.y * 2.0f),
		k2d::vi2d(scaled_ui.x * 0.25f-2, scaled_ui.y * 0.25f - 2),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Hi", 0.10f, 25.0f));
	fps_button_high->SetActive(true);
	fps_button_high->SetTextOffset(k2d::vf2d(-tile_size.x * 0.25f, -tile_size.y * 0.2f));
	fps_button_high->AddCallbackFunction(this, &ConquestLocal::SetTargetFpsHigh);
	ui_buttons.push_back(fps_button_high);

	UIButton* fps_button_unlimited = new UIButton("FPSButtonUnlimited",
		k2d::vi2d(0 + map_size.x * tile_size.x + tile_size.x * 4.0f, map_size.y * tile_size.y + tile_size.y * 2.0f),
		k2d::vi2d(scaled_ui.x * 0.25f-2, scaled_ui.y * 0.25f - 2),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("U", 0.10f, 25.0f));
	fps_button_unlimited->SetActive(true);
	fps_button_unlimited->SetTextOffset(k2d::vf2d(-tile_size.x * 0.25f, -tile_size.y * 0.2f));
	fps_button_unlimited->AddCallbackFunction(this, &ConquestLocal::SetTargetFpsUnlimited);
	ui_buttons.push_back(fps_button_unlimited);

#pragma endregion buttons

	

#pragma region Graphs

	/*
		GRAPHS
	*/
	// Generation history graph
	generation_history = new UIGraph("GenHistory",
		k2d::vi2d(0 -tile_size.x * 0.5f + scaled_ui.x * 2 + tile_size.x * 2, -scaled_ui.y * 1 - tile_size.y * 2),
		k2d::vi2d(scaled_ui.x * 5, scaled_ui.y * 2),
		25.0f,
		100, 1200, // max points, max_value
		load_texture_from_cache("full"), sprite_batch);
	generation_history->AddHorizontalLine(0.5f, k2d::Color(255, 0, 0, 128));
	generation_history->SetBackground(k2d::Color(40, 255));
	generation_history->AddTrendLine(k2d::Color(255, 0, 255, 255));
	generation_history->AddText(create_text("Generation Average Fitness", k2d::vi2d(generation_history->GetPosition() + k2d::vf2d(-scaled_ui.x * 2.5f, scaled_ui.y * 0.8f)), 0.12f, 24.9f));

	// Current gen tiles owned histogram
	current_gen_tiles_owned_histogram = new UIGraph("CurrentGenTilesOwned",
		k2d::vi2d(0 - tile_size.x * 0.5f + scaled_ui.x * 3 + scaled_ui.x * 4 + tile_size.x * 2, -scaled_ui.y * 1 - tile_size.y * 2),
		k2d::vi2d(scaled_ui.x * 5, scaled_ui.y * 2),
		25.0f,
		200, 1200, // max_points, max_value
		load_texture_from_cache("full"), sprite_batch);
	current_gen_tiles_owned_histogram->AddHorizontalLine(0.5f, k2d::Color(255, 0, 0, 128));
	current_gen_tiles_owned_histogram->SetBackground(k2d::Color(20, 255));
	current_gen_tiles_owned_histogram->AddTrendLine(k2d::Color(255, 0,255, 255));
	current_gen_tiles_owned_histogram->AddText(create_text("Fitness", k2d::vi2d(current_gen_tiles_owned_histogram->GetPosition() + k2d::vf2d(-scaled_ui.x * 2.5f, scaled_ui.y * 0.8f)), 0.12f, 24.9f));


	// Current gen tiles owned histogram
	pick_chance_graph = new UIClickableGraph("PickChanceGraph",
		k2d::vi2d(0 - scaled_ui.x * 2 - tile_size.x * 1.5f, tile_size.y * map_size.y * 0.5f + scaled_ui.y * 0.5f - tile_size.y * 0.5f),
		k2d::vi2d(scaled_ui.x * 2.5f, scaled_ui.y * 2),
		25.0f,
		std::lround(population_size * top_percentile),
		find_max_local(0, std::lround(population_size * top_percentile)), // max_data_value
		load_texture_from_cache("full"), sprite_batch);
	pick_chance_graph->SetBackground(k2d::Color(20, 255));
	pick_chance_graph->AddText(create_text("Selection chance", k2d::vi2d(pick_chance_graph->GetPosition() + k2d::vf2d(-scaled_ui.x * 1.25f, scaled_ui.y * 0.8f)), 0.12f, 24.9f));

#pragma endregion

#pragma region ProgressBars

	UIProgressBar* generation_progressbar = new UIProgressBar("GenerationProgressBar",
		k2d::vf2d(0 - tile_size.x * 0.5f, tile_size.y * map_size.y),
		k2d::vf2d(tile_size.x * map_size.x, tile_size.y * 0.25f),
		25.0f,
		load_texture_from_cache("full"),
		sprite_batch);

	generation_progressbar->AddProgressValue(&last_played_index);
	generation_progressbar->AddTargetValue(&population_size);
	generation_progressbar->AddBackground(k2d::Color(64, 255));
	ui_progressbars.push_back(generation_progressbar);

#pragma endregion

#pragma region ScoreBar

	scorebar = new UIScoreBar("ScoreBar", k2d::vf2d(-tile_size.x *0.5f + tile_size.x * map_size.x * 0.5f, tile_size.y * map_size.y + tile_size.y * 2.5f),
		k2d::vf2d(tile_size.x * map_size.x, tile_size.y * 2.0f),
		25.0f,
		load_texture_from_cache("full"),
		sprite_batch);
	scorebar->AddBackground(k2d::Color(64, 255));
	scorebar->SetMaxTileCount(map_size.x * map_size.y);
	scorebar->SetVariablePointers(&p0_tiles, &p0_color, &p1_tiles, &p1_color);
	scorebar->AddMarker(&half_of_tiles, k2d::Color(255, 255, 0, 128));
	scorebar->AddMarker(&average_score_this_generation, k2d::Color(255, 0,0, 128));
	scorebar->AddMarker(&previous_tiles_owned, k2d::Color(0, 0, 255, 128));

#pragma endregion

#pragma region ParentIdsList

	parent_ids_list = new UIList("ParentIdsList", 
		k2d::vi2d(0 - scaled_ui.x * 5 + tile_size.x * 2.5, +scaled_ui.y * 4.5f - tile_size.y * 1.5),
		k2d::vi2d(-scaled_ui.x * 0.5f, -5),
		k2d::vi2d(scaled_ui.x * 1, scaled_ui.y * 8.0f + tile_size.y * 2),
		25.0f, tile_size.x, 32, 
		load_texture_from_cache("full"), sprite_batch, font1, 0.12f, k2d::Color(255));
	parent_ids_list->AddBackground(k2d::Color(255));
#pragma endregion

#pragma region Rectangles

	nn_vision_rect = new UIRectangle("VisionRect", 0, 0, 30.0f, CreateDefaultSprite("full", k2d::Color(128, 128, 0, 192)));

#pragma endregion


	// Insert all the freshly created ui elements into this vector
	all_of_the_ui.insert(all_of_the_ui.end(), ui_buttons.begin(), ui_buttons.end());
	all_of_the_ui.insert(all_of_the_ui.end(), ui_clickable_labels.begin(), ui_clickable_labels.end());
	all_of_the_ui.insert(all_of_the_ui.end(), ui_multilabels.begin(), ui_multilabels.end());
	all_of_the_ui.insert(all_of_the_ui.end(), ui_progressbars.begin(), ui_progressbars.end());
	all_of_the_ui.push_back(generation_history);
	all_of_the_ui.push_back(current_gen_tiles_owned_histogram);
	all_of_the_ui.push_back(pick_chance_graph);
	all_of_the_ui.push_back(scorebar);
	all_of_the_ui.push_back(parent_ids_list);
	all_of_the_ui.push_back(nn_vision_rect);

	return 0;
}

// returns the winners id
int ConquestLocal::PlayGame(AI* first, AI* second)
{
	game_in_progress = true;

	SimpleAI* tmp1 = dynamic_cast<SimpleAI*>(first);
	if (tmp1)
	{
		tmp1->SetMapSize(server_sim->GetMapSize());
		tmp1->SetStartingPosition(server_sim->GetStartingPositions()[0]);
		tmp1->SetCurrentColorOwned(0);
	}

	SimpleAI* tmp2 = dynamic_cast<SimpleAI*>(second);
	if (tmp2)
	{
		tmp2->SetMapSize(server_sim->GetMapSize());
		tmp2->SetStartingPosition(server_sim->GetStartingPositions()[1]);
		tmp2->SetCurrentColorOwned(1);
	}

	server_sim->ConnectToServer(first->GetClientId());
	server_sim->ConnectToServer(second->GetClientId());

	// Set the first players id as the turn player id
	first->SetCurrentTurnPlayersId(first->GetClientId());
	second->SetCurrentTurnPlayersId(first->GetClientId());

	// Agents are in game
	first->SetInGame(true);
	second->SetInGame(true);

	// First goes first etc.
	first->SetWhichPlayerAmI(0);
	second->SetWhichPlayerAmI(1);


	server_sim->StartGame(map_index);
	// adds one after the game
	map_index++;

	return 0;
}

void ConquestLocal::PreRender()
{
	k2d::Application::PreRender();
}

void ConquestLocal::Update()
{
	if (!server_sim->GetGameInProgress())
	{
		server_sim->DisconnectFromServer(ai_agents.at(last_played_index)->GetClientId());
		server_sim->DisconnectFromServer(opponent->GetClientId());

		if (map_index >= num_maps)
		{
			CheckIfBestOfGeneration();
			CalculateGenerationAverage();
			SetPreviousIdAndTileCount();
			UpdateProgressBarValues();
			UpdateParentIdsListValues();
			current_gen_tiles_owned_histogram->AddDataPoint(previous_tiles_owned);
			
			map_index = 0;
			last_played_index++;
		}

		// One epoch done
		if (last_played_index >= ai_agents.size())
		{
			// 
			SaveGeneticAlgorithmVariablesToFile("Data/Gen.json");
			GeneticAlgorithm();
			last_played_index = 0;

			current_best_of_gen_id = 0;
			current_best_of_gen_tiles_owned = 0;

			generation_history->AddDataPoint(average_score_this_generation);
			average_score_this_generation = 0;

			// Create a new map after the generation has played their games
			if (should_create_new_map)
			{
				should_create_new_map = false;
				UIToggleButton* map_button = static_cast<UIToggleButton*> (get_button_by_name("NewMapButton"));
				if (map_button)
				{
					map_button->ResetToUntoggledState();
				}
				server_sim->CreateNewMaps();
			}
		}

		// g for bad, h for simple ai
		if (bad_ai_enabled == true)
		{
			opponent = bad_ai;
		}
		else
		{
			opponent = simple_ai;
		}

		// Running agent vs. simpleAI
		PlayGame(ai_agents.at(last_played_index), opponent);
	}

	if (!paused)
	{
		server_sim->Update();

		for (AI* ai : ai_agents)
		{
			// Remove later TODO
			if (ai->GetInGame())
			{
				ai->Update();
			}
		}

		opponent->Update();
	}

	// Handle events
	Event e = server_sim->GetNextEventFromQueue(spectator_id);
	HandleEvent(e);
	
	tilemap = server_sim->GetBoardState();
	players = server_sim->GetPlayers();


	if (ui_enabled)
	{
		UpdateTileColors();
		UpdateScoreboardIds();
		UpdateScoreboardIds();
		UpdateScorebarValues();
		UpdateDebugRectanglePosition();
	}

	ClampGeneticAlgorithmVariables();



	if (ui_enabled)
	{
		for (GameObject* tile : tiles)
		{
			// Draw tile
			tile->Update(dt);
		}

		for (UIBase* ui : all_of_the_ui)
		{
			ui->Update(dt);
		}
	}

	update_input();
	//engine->SetWindowTitle("Conquest AI Training. fps: " + std::to_string(engine->GetCurrentFPS()));

	k2d::Application::Update();
}


void ConquestLocal::GeneticAlgorithm()
{

	UIList* l = static_cast<UIList*> (get_ui_by_name("ParentIdsList"));

	l->SetVectorToFollow(nullptr);
	l->UpdateListValues();

	// Sort best first
	std::sort(ai_agents.begin(), ai_agents.end(), [](AI* a, AI* b) -> bool
	{
		return a->GetFitness() > b->GetFitness();
	});

	int cutoff_index = std::lround(top_percentile * population_size);
	if (cutoff_index < ai_agents.size() )
	{
		// Pick top some % for breeding
		for (size_t i = cutoff_index; i < ai_agents.size(); i++)
		{
			delete ai_agents[i];
		}
		ai_agents.erase(ai_agents.begin() + cutoff_index, ai_agents.end());
	}

	UpdateSelectionWeights();

	int max_index = ai_agents.size() - 1;
	std::discrete_distribution<int> distribution(selection_weights.begin(), selection_weights.begin()+ max_index);
	ai_agents.reserve(population_size);
	// Breed new AIs until we have the original amount of agents
	while (ai_agents.size() < population_size)
	{
		int index1 = distribution(random_engine);

		NeuralAI* tmp1 = static_cast<NeuralAI*>(ai_agents[index1]);

		NeuralAI* child = new NeuralAI(*tmp1, running_agent_id++, server_sim);

		float topology_mutation = Random::get(0.0f, 1.0f);
		if (topology_mutation < topology_mutation_chance)
		{
			child->MutateTopology(topology_mutation_rate);
		}

		float sight_size_mutation = Random::get(0.0f, 1.0f);
		if (sight_size_mutation < sight_size_mutation_rate)
		{
			child->MutateSightSize(sight_size_mutation_epsilon);
		}

		float playstyle_mutation = Random::get(0.0f, 1.0f);
		if (playstyle_mutation < playstyle_mutation_rate)
		{
			child->MutatePlaystyle(playstyle_mutation_epsilon);
		}

		// Randomize the mutation type
		float close_mutation = Random::get(0.0f, 1.0f);
		if (close_mutation < mutation_type_chance)
		{
			// Mutates the child close to the parent
			child->CloseMutate(close_mutation_rate, close_mutation_epsilon);
		}
		else
		{
			// Mutate the agent randomly
			child->Mutate(mutation_rate);
		}

		// Push the child into the pool
		ai_agents.push_back(child);
	}

	for (AI* agent : ai_agents)
	{
		agent->SetTilesOwned(0);
		agent->SetFitness(0);
	}

	k2d::KUSI_DEBUG("\n\n\n\n\n EPOCH %i DONE \n\n\n\n\n", epoch++);

	return;
}

void ConquestLocal::update_input()
{
	
	if (engine->GetInputManager().IsButtonPressedThisFrame(SDL_BUTTON_LEFT) || engine->GetInputManager().IsButtonPressed(SDL_BUTTON_RIGHT))
	{
		// Check which button is pressed.
		k2d::vi2d click_pos = engine->ScreenToWorld(engine->GetMouseCoords());
		std::cout << "click: " << click_pos << "\n";

		// Depth checking for click
		std::vector<UIBase*> temp_clicked;

		for (UIBase* ui : all_of_the_ui)
		{
			if (ui->IsActive())
			{
				UIClickable* c = dynamic_cast<UIClickable*> (ui);
				if (c)
				{
					float dx = 0.0f;
					float dy = 0.0f;
					// BOt left position
					k2d::vf2d button_pos;
					button_pos.x = ui->GetPosition().x - ui->GetSize().x / 2;
					button_pos.y = ui->GetPosition().y - ui->GetSize().y / 2;
					k2d::vf2d button_dims;
					button_dims.x = ui->GetSize().x;
					button_dims.y = ui->GetSize().y;

					dx = click_pos.x - button_pos.x;
					dy = click_pos.y - button_pos.y;

					// Check if its a hit
					if (dx > 0 && dx < button_dims.x
						&& dy > 0 && dy < button_dims.y)
					{
						temp_clicked.push_back(ui);
						//c->OnClick(k2d::vf2d(dx, dy));
					}
				}
			}
		}

		// Sort highest (closest to user) first
		std::sort(temp_clicked.begin(), temp_clicked.end(), [](UIBase* a, UIBase* b) -> bool 
			{
				return a->GetDepth() > b->GetDepth();
			});

		if (!temp_clicked.empty())
		{
			UIClickable* click_this = dynamic_cast<UIClickable*>(temp_clicked[0]);
			if (click_this)
			{
				float dx = 0.0f;
				float dy = 0.0f;

				k2d::vf2d button_pos;
				button_pos.x = temp_clicked[0]->GetPosition().x - temp_clicked[0]->GetSize().x / 2;
				button_pos.y = temp_clicked[0]->GetPosition().y - temp_clicked[0]->GetSize().y / 2;
				k2d::vf2d button_dims;
				button_dims.x = temp_clicked[0]->GetSize().x;
				button_dims.y = temp_clicked[0]->GetSize().y;

				dx = click_pos.x - button_pos.x;
				dy = click_pos.y - button_pos.y;

				click_this->OnClick(k2d::vf2d(dx, dy));
			}
		}

	}

	if (engine->GetInputManager().IsMouseWheelScrolledThisFrame(k2d::WheelDirection::UP))
	{
		variable_change_multiplier /= 10;
		k2d::clamp(variable_change_multiplier, 1, 1000000);
		for (UIClickableLabel* l : ui_clickable_labels)
		{
			l->SetVariableMultiplier(variable_change_multiplier);
		}
	}
	if (engine->GetInputManager().IsMouseWheelScrolledThisFrame(k2d::WheelDirection::DOWN))
	{
		variable_change_multiplier *= 10;
		k2d::clamp(variable_change_multiplier, 1, 1000000);
		for (UIClickableLabel* l : ui_clickable_labels)
		{
			l->SetVariableMultiplier(variable_change_multiplier);
		}
	}

	// Disable all rendering /updating
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_c))
	{
		ui_enabled = !ui_enabled;
	}


	// Raise target fps
	if (engine->GetInputManager().IsKeyPressed(SDLK_F2))
	{
		fps_target += 1.0f;
		engine->SetTargetFps(fps_target);
	}
	// LOwer target fps
	if (engine->GetInputManager().IsKeyPressed(SDLK_F3))
	{
		fps_target -= 1.0f;
		engine->SetTargetFps(fps_target);
	}
#pragma region Colors


	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_1))
	{
		GetRandomColorFromLoadedSkins(0);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_2))
	{
		GetRandomColorFromLoadedSkins(1);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_3))
	{
		GetRandomColorFromLoadedSkins(2);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_4))
	{
		GetRandomColorFromLoadedSkins(3);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_5))
	{
		GetRandomColorFromLoadedSkins(4);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_6))
	{
		GetRandomColorFromLoadedSkins(5);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_7))
	{
		GetRandomColorFromLoadedSkins(6);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_8))
	{
		GetRandomColorFromLoadedSkins(7);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_9))
	{
		GetRandomColorFromLoadedSkins(8);
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_0))
	{
		GetRandomColorFromLoadedSkins(9);
	}
	// Random ALL skins
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_k))
	{
		
		std::uniform_int_distribution<int> rand_col(0, loaded_skins.size() - 1);

		for (size_t i = 0; i < skins.size(); i++)
		{
			skins.at(i) = loaded_skins.at(rand_col(random_engine));
		}
		UpdateTileColors();
	}
#pragma endregion
}

void ConquestLocal::UpdateTileColors()
{
	UpdateTileBrightness();
	if (tiles.size() < map_size.x * map_size.y)
	{
		// Clear tiles, and create new tiles to replace them
		for (GameObject* tile : tiles)
		{
			delete tile;
		}
		tiles.clear();
		for (uint8_t y = 0; y < map_size.y; y++)
		{
			for (uint8_t x = 0; x < map_size.x; x++)
			{
				tiles.push_back(new GameObject(GridToWorldPos(k2d::vi2d(x, y)), tile_size.x, tile_size.y,
					create_tile_sprite("full", skins.at(tilemap[y][x].color))));
			}
		}
	}
	else
	{
		// If we have all the tiles already here, just change their colors
		for (uint8_t y = 0; y < map_size.y; y++)
		{
			for (uint8_t x = 0; x < map_size.x; x++)
			{
				tiles.at(y * map_size.x + x)->GetSprite()->SetColor(skins.at(tilemap[y][x].color));
			}
		}
	}
}

void ConquestLocal::UpdateScoreboardIds()
{

	for (size_t i = 0; i < players.size(); i++)
	{
		if (players[i].id >= 0)
		{
			if (i == 0)
			{
				p0_id = players[i].id;
				UIMultiLabel* ml = static_cast<UIMultiLabel*>(get_ui_by_name("P0Scoreboard"));
				if (ml)
				{
					ml->AddBackground(skins[players[i].num_owned]);
				}
			}
			else if (i == 1)
			{
				p1_id = players[i].id;
				UIMultiLabel* ml = static_cast<UIMultiLabel*>(get_ui_by_name("P1Scoreboard"));
				if (ml)
				{
					ml->AddBackground(skins[players[i].num_owned]);
				}
			}
		}
	}
}

void ConquestLocal::CalculateNewSelectionWeights()
{
	ClampWeightSelectionVariables();
	selection_weights.resize(std::lround(population_size * top_percentile));
	for (size_t i = 0; i < selection_weights.size(); i++)
	{
		// Good looking function
		selection_weights.at(i) = weight_selection_function(i, weight_selection_a, weight_selection_b);
	}
}

void ConquestLocal::UpdateSelectionWeights()
{
	selection_weights.resize(std::lround(population_size * top_percentile));
	
	pick_chance_graph->SetMaxDataValue(find_max_local(0, std::lround(population_size * top_percentile)));
	pick_chance_graph->SetDataToFollow(&selection_weights);
}

void ConquestLocal::IncreaseWeightSlope()
{
	weight_selection_a += 10.0f;
	weight_selection_b -= 1.0f;
}

void ConquestLocal::ResetWeightSlope()
{
	weight_selection_a = 200.0f;
	weight_selection_b = 10.0f;
}

void ConquestLocal::DecreaseWeightSlope()
{
	weight_selection_a -= 10.0f;
	weight_selection_b += 1.0f;
}

void ConquestLocal::PauseGame()
{
	paused = !paused;
}

void ConquestLocal::ToggleMapCreation()
{
	should_create_new_map = !should_create_new_map;
}

void ConquestLocal::ToggleOpponentType()
{
	bad_ai_enabled = !bad_ai_enabled;
}

void ConquestLocal::ClampGeneticAlgorithmVariables()
{
	k2d::clamp(top_percentile, 0.01f, 1.0f);

	k2d::clamp(mutation_rate, 0.00f, 1.0f);
	k2d::clamp(close_mutation_rate, 0.0f, 1.0f);
	k2d::clamp(close_mutation_epsilon, 0.000001, 1.0);

	k2d::clamp(mutation_type_chance, 0.0f, 1.0f);
	k2d::clamp(topology_mutation_chance, 0.0f, 1.0f);
	k2d::clamp(topology_mutation_rate, 0.0f, 1.0f);

	k2d::clamp(playstyle_mutation_rate, 0.0f, 1.0f);
	k2d::clamp(playstyle_mutation_epsilon, 0.0f, 1.0f);

	k2d::clamp(sight_size_mutation_rate, 0.0f, 1.0f);
	k2d::clamp(sight_size_mutation_epsilon, 0, 10);

	k2d::clamp(num_layers_mutation_chance, 0.0f, 1.0f);

	k2d::clamp(population_size, 1, 100000);
	k2d::clamp(num_maps, 1, 1000);
}

void ConquestLocal::ClampWeightSelectionVariables()
{
	k2d::clamp(weight_selection_a, 1.0f, 100000.0f);
	k2d::clamp(weight_selection_b, 1.0f, 100000.0f);
}

void ConquestLocal::ClampTileBrightness()
{
	k2d::clamp(tile_brightness, 0.0f, 1.0f);
}

void ConquestLocal::UpdateTileBrightness()
{
	for (size_t i = 0; i < 10; i++)
	{
		skins.at(i).a = floor((float)255 * tile_brightness);
	}
}

void ConquestLocal::SetTargetFpsLow()
{
	engine->SetTargetFps(1.0f);
}

void ConquestLocal::SetTargetFpsMed()
{
	engine->SetTargetFps(10.0f);
}

void ConquestLocal::SetTargetFpsHigh()
{
	engine->SetTargetFps(60.0f);
}

void ConquestLocal::SetTargetFpsUnlimited()
{
	engine->SetTargetFps(100000.0f);
}

void ConquestLocal::UpdateScorebarValues()
{
	p0_tiles = players.at(0).tiles_owned;
	p0_color = k2d::Color(skins.at(players.at(0).num_owned));
	p1_tiles = players.at(1).tiles_owned;
	p1_color = k2d::Color(skins.at(players.at(1).num_owned));
	UIScoreBar* ui = static_cast<UIScoreBar*>(get_ui_by_name("ScoreBar"));
	if (ui)
	{
		ui->UpdateBar();
	}
}

void ConquestLocal::UpdateProgressBarValues()
{
	for (UIProgressBar* b : ui_progressbars)
	{
		b->UpdateProgressBarValues();
	}
}

void ConquestLocal::UpdateParentIdsListValues()
{
	UIList* l = static_cast<UIList*> (get_ui_by_name("ParentIdsList"));
	NeuralAI* ai = static_cast<NeuralAI*> (ai_agents.at(last_played_index));

	l->SetVectorToFollow(ai->GetParentIds());
	l->UpdateListValues();
}

void ConquestLocal::UpdateDebugRectanglePosition()
{
	UIRectangle* l = static_cast<UIRectangle*> (get_ui_by_name("VisionRect"));
	NeuralAI* ai = static_cast<NeuralAI*> (ai_agents.at(last_played_index));

	int sight_size = ai->GetSightSize();
	if (sight_size % 2 == 0)
	{
		l->SetPosition(ai->GetCurrentVisionPosition() * tile_size.x - (tile_size.x * 0.5));
	}
	else
	{
		l->SetPosition(ai->GetCurrentVisionPosition() * tile_size.x);
	}

	l->SetSize(k2d::vi2d(sight_size * tile_size));
}

void ConquestLocal::CalculateGenerationAverage()
{
	int sum = 0;
	for (size_t i = 0; i < ai_agents.size(); i++)
	{
		sum += ai_agents[i]->GetFitness();
	}

	average_score_this_generation = sum / (last_played_index + 1);
}

void ConquestLocal::CheckIfBestOfGeneration()
{
	if (ai_agents.at(last_played_index)->GetFitness() > current_best_of_gen_tiles_owned)
	{
		current_best_of_gen_id = ai_agents.at(last_played_index)->GetClientId();
		current_best_of_gen_tiles_owned = ai_agents.at(last_played_index)->GetFitness();
	}
}

void ConquestLocal::SetPreviousIdAndTileCount()
{
	previous_id = ai_agents[last_played_index]->GetClientId();
	previous_tiles_owned = ai_agents[last_played_index]->GetFitness();
}

void ConquestLocal::SaveGeneticAlgorithmVariablesToFile(std::string file_name)
{
	using json = nlohmann::json;
	
	/*
	k2d::clamp(top_percentile, 0.01f, 1.0f);

	k2d::clamp(mutation_rate, 0.00f, 1.0f);
	k2d::clamp(close_mutation_rate, 0.0f, 1.0f);
	k2d::clamp(close_mutation_epsilon, 0.000001, 1.0);

	k2d::clamp(mutation_type_chance, 0.0f, 1.0f);
	k2d::clamp(topology_mutation_chance, 0.0f, 1.0f);
	k2d::clamp(num_layers_mutation_chance, 0.0f, 1.0f);

	k2d::clamp(population_size, 1, 100000);
	*/
	json j;

	j["population_size"] = population_size;
	j["top_percentile"] = top_percentile;
	j["mutation_rate"] = mutation_rate;
	j["close_mutation_rate"] = close_mutation_rate;
	j["close_mutation_epsilon"] = close_mutation_epsilon;
	j["mutation_type_chance"] = mutation_type_chance;
	j["topology_mutation_chance"] = topology_mutation_chance;
	j["topology_mutation_rate"] = topology_mutation_rate;
	j["num_layers_mutation_chance"] = num_layers_mutation_chance;
	j["playstyle_mutation_rate"] = playstyle_mutation_rate;
	j["playstyle_mutation_epsilon"] = playstyle_mutation_epsilon;
	j["sight_size_mutation_rate"] = sight_size_mutation_rate;
	j["sight_size_mutation_epsilon"] = sight_size_mutation_epsilon;
	j["num_maps"] = num_maps;

	std::ofstream file;
	file.open(file_name);
	file << std::setw(4) << j << std::endl;
	file.close();

	return;
}

void ConquestLocal::GetRandomColorFromLoadedSkins(int index)
{
	std::uniform_int_distribution<int> rand_col(0, loaded_skins.size() - 1);

	skins.at(index) = loaded_skins.at(rand_col(random_engine));

	UpdateTileColors();
}

int ConquestLocal::bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x_, uint8_t y_)
{
	int num_visited = 0;
	// Visited array
	uint8_t v[100][100];
	memset(v, 0, sizeof(v));
	

	std::queue<std::pair<uint8_t, uint8_t>> the_queue;

	the_queue.push({ x_, y_ });
	v[y_][x_] = 1;


	// run until the queue is empty
	while (!the_queue.empty())
	{
		// Extrating front pair
		std::pair<uint8_t, uint8_t> coord = the_queue.front();
		int x = coord.first;
		int y = coord.second;

		num_visited++;

		// Poping front pair of queue
		the_queue.pop();

		// Down
		if (valid_tile(x, y + 1, map_size)
			&& v[y + 1][x] == 0
			&& ((tilemap[y + 1][x].color == our_color && tilemap[y + 1][x].owner == owner) || tilemap[y + 1][x].color == new_color))
		{
			the_queue.push({ x, y + 1 });
			v[y + 1][x] = 1;
		}
		// Up
		if (valid_tile(x, y - 1, map_size)
			&& v[y - 1][x] == 0
			&& ((tilemap[y - 1][x].color == our_color && tilemap[y - 1][x].owner == owner) || tilemap[y - 1][x].color == new_color))
		{
			the_queue.push({ x, y - 1 });
			v[y - 1][x] = 1;
		}
		// Right
		if (valid_tile(x + 1, y, map_size)
			&& v[y][x + 1] == 0
			&& ((tilemap[y][x + 1].color == our_color && tilemap[y][x + 1].owner == owner) || tilemap[y][x + 1].color == new_color))
		{
			the_queue.push({ x + 1, y });
			v[y][x + 1] = 1;
		}
		// Left
		if (valid_tile(x - 1, y, map_size)
			&& v[y][x - 1] == 0
			&& ((tilemap[y][x - 1].color == our_color && tilemap[y][x - 1].owner == owner) || tilemap[y][x - 1].color == new_color))
		{
			the_queue.push({ x - 1, y });
			v[y][x - 1] = 1;
		}
	}

	return num_visited;
}

k2d::Sprite* ConquestLocal::create_tile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), tile_size.x, tile_size.y, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}

k2d::Sprite* ConquestLocal::CreateDefaultSprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), scaled_ui.x, scaled_ui.y, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}

k2d::Sprite* ConquestLocal::create_projectile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), 8, 8, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}


k2d::Text* ConquestLocal::create_text(std::string text, float scale, float depth)
{
	return new k2d::Text(text, font1, 0, 0, scale, depth, k2d::Color(255,255, 0, 255), sprite_batch);
}

k2d::Text* ConquestLocal::create_text(std::string text, k2d::vi2d position, float scale, float depth)
{
	return new k2d::Text(text, font1, position.x, position.y, scale, depth, k2d::Color(255), sprite_batch);
}

UIBase* ConquestLocal::get_ui_by_name(std::string name)
{
	for (UIBase* ui : all_of_the_ui)
	{
		if (name == ui->GetName())
		{
			return ui;
		}
	}
	return nullptr;
}

UIButton* ConquestLocal::get_button_by_name(std::string name)
{
	for (UIButton* ui : ui_buttons)
	{
		if (name == ui->GetName())
		{
			return ui;
		}
	}
	return nullptr;
}

void ConquestLocal::CreateNewMaps()
{
	server_sim->SetNumMaps(num_maps);
	server_sim->CreateNewMaps();
}

void ConquestLocal::HandleEvent(Event& e)
{
	switch (e.GetType())
	{
	case EventType::GAME_OVER:
	{
		// If the game is over, create a database entry for the match
		std::string data = e.GetData();

		std::string delimiter = ":";

		size_t pos = 0;
		std::string token;
		std::vector<std::string> tokens;
		while ((pos = data.find(delimiter)) != std::string::npos) {
			token = data.substr(0, pos);
			tokens.push_back(token);
			data.erase(0, pos + delimiter.length());
		}

		int winner_id = (std::stoi(tokens[0]));
		int turns_played = (std::stoi(tokens[1]));
		int match_id = std::stoi(tokens[2]);
		int p0_id = std::stoi(tokens[3]);
		int p1_id = std::stoi(tokens[4]);
		int p0_tiles_owned = std::stoi(tokens[5]);
		int p1_tiles_owned = std::stoi(tokens[6]);
		std::string encoded_turn_history = tokens[7];
		std::string initial_board_state = data;

		//db_handler->InsertMatchData(match_id, winner_id, turns_played, p0_id, p1_id, encoded_turn_history, initial_board_state);

		// Store the Match id
		// Store the winners id 
		// Store the num_turns

		break;
	}
	case EventType::TURN_CHANGE:
	{
		std::string data = e.GetData();
		// turn agents id = std::stoi(data));

		taken_colors = server_sim->GetTakenColors();
		turns_played = server_sim->GetTurnsPlayed();

		break;
	}
	case EventType::INVALID_COLOR:
	{

		break;
	}
	default:
		break;
	}
}


float ConquestLocal::find_max_local(int first, int last)
{
	// Loop from first to one before last, record highest value
	float max = -9999999.0f;
	for (size_t i = first; i < last; i++)
	{
		// send the iterative number to the function, collect the output and compare to current max
		if (max < weight_selection_function(i, weight_selection_a, weight_selection_b))
		{
			max = weight_selection_function(i, weight_selection_a, weight_selection_b);
		}
	}
	return max;
}