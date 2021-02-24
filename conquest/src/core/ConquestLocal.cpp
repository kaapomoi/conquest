#include <core/ConquestLocal.h>

ConquestLocal::ConquestLocal() :
	// Init with mapsize, colors
	server_sim(k2d::vi2d(40, 30), 6)
{
	window_title = "Conquest AI Training";
	window_width = 1200;
	window_height = 900;
	v_sync = false;

	face = 0;
	ft = 0;

	fps_target = 60.0f;
	dt = 0.0000000001;

	camera_mvmt_speed = 200.f;

	spectator_id = -9999;

	// Session id from time from 1-1-2000
	time_t t;
	struct tm y2k = { 0 };
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
	time(&t);
	int ses_id = difftime(t, mktime(&y2k));


	db_dir = "Data/TEST.db";
	db_handler = new DatabaseHandler(ses_id, db_dir);

	db_handler->CreateMatchesTable();

	if (init_engine() == 0)
	{
		// Init game
		InitGeneticAlgorithmValues();
		init_game();
		create_ai();
		//
		create_ui();
		//create_ui_unit_card();

		CalculateNewSelectionWeights();

		UpdateSelectionWeights();
		

		// run game
		run();
	}
}

ConquestLocal::~ConquestLocal()
{
}

int ConquestLocal::init_engine()
{
	engine = new k2d::Engine(window_title, window_width, window_height, v_sync);
	sprite_batch = engine->GetSpriteBatch();
	engine->AddShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag", "2d",
		{ "vertex_position", "vertex_color", "vertex_uv" });

	return 0;
}

int ConquestLocal::init_game()
{
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


	origin = k2d::vi2d(0, 0);

	engine->SetCameraPosition(k2d::vi2d(window_width / 4, window_height / 4));

	font1 = LoadFont("Fonts/opensans.ttf");
 
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

	// Default skins:
	// red
	// green
	// blue
	// yellow
	// magenta

	// Send packets every 33ms = 30hz
	timer_counter = 0.0f;
	// TODO set this somehow
	map_size = server_sim.GetMapSize();
	num_colors = server_sim.GetTakenColors().size();

	// Net code
	random_engine.seed((unsigned int) time(NULL));

	return 0;
}

void ConquestLocal::InitGeneticAlgorithmValues()
{
	// 0.1f = 10%;
	top_percentile = 0.2f;

	mutation_rate = 0.01f;
	close_mutation_rate = 0.05f;
	close_mutation_epsilon = 0.50;

	mutation_type_chance = 0.95f;
	variable_change_multiplier = 1;
}

int ConquestLocal::create_ai()
{
	population_size = 100;

	bad_ai = new BadAI(0, &server_sim);
	simple_ai = new SimpleAI(1, &server_sim);

	opponent = bad_ai;
	bad_ai_enabled = true;

	running_agent_id = 2;
	last_played_index = -1;

	for (size_t i = 0; i < population_size; i++)
	{
		ai_agents.push_back(new NeuralAI(running_agent_id++, &server_sim));
	}

	return 0;
}

int ConquestLocal::create_ui()
{
	scaled_ui = tile_size * 4;

	ui_enabled = true;

	// Generation texts
	UIElement* gen = new UIElement("Generation", k2d::vi2d(0 - scaled_ui.x * 2, tile_size.y * map_size.y - scaled_ui.y * 0.5f - tile_size.y * 0.5f),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x * 3 + tile_size.x, scaled_ui.y * 3, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255), load_texture_from_cache("full"), sprite_batch),
		create_text("Gen: 0", 0.15f, 25.0f));
	gen->SetIsActive(true);
	gen->SetTextOffset(k2d::vf2d(-scaled_ui.x * 1.5f - tile_size.x * 0.5f, scaled_ui.y * 1.5f - tile_size.y));

	gen->AddChild(new UIElement("BestOfThisGen", k2d::vi2d(0 - scaled_ui.x * 2, tile_size.y * map_size.y - scaled_ui.y * 0.5f - tile_size.y * 0.5f), 
		0,
		create_text("Best of gen: ", 0.15f, 25.0f)));
	gen->GetChild()->SetIsActive(true);
	gen->GetChild()->SetTextOffset(k2d::vf2d(-scaled_ui.x * 1.5f - tile_size.x * 0.5f, scaled_ui.y * 1.5f - tile_size.y * 2));
	ui_elements.push_back(gen);
	
	// Previous players id, tiles owned
	UIElement* previous_text = new UIElement("PreviousText", k2d::vi2d(0 - scaled_ui.x * 2, tile_size.y * map_size.y - scaled_ui.y * 0.5f - tile_size.y * 0.5f),
		0,
		create_text("Previous: ", 0.15f, 25.0f));
	previous_text->SetIsActive(true);
	previous_text->SetTextOffset(k2d::vf2d(-scaled_ui.x * 1.5f - tile_size.x * 0.5f, scaled_ui.y - tile_size.y));
	ui_elements.push_back(previous_text);

	// Opponent toggle button
	UIElement* opponent_choice = new UIElement("OpponentChoice", k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y / 2, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255), load_texture_from_cache("full"), sprite_batch),
		create_text(" Bad    Simple", 0.10f, 25.0f));
	opponent_choice->SetIsButton(true);
	opponent_choice->SetIsActive(true);
	opponent_choice->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, 0));

	opponent_choice->AddChild(new UIElement("OpponentChoiceDarkout", k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - tile_size.x / 2, tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x / 2, scaled_ui.y / 2, 21.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0,0,0, 128), load_texture_from_cache("full"), sprite_batch),
		0));

	ui_elements.push_back(opponent_choice);


	// multiplier label
	UIClickableLabel* multiplier = new UIClickableLabel("VariableChangeMultiplier", "Mult.: ",
		k2d::vi2d(0 - scaled_ui.x * 2 - tile_size.x * 2, tile_size.y * 0.5f +scaled_ui.y * 0.5f + 1),
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
		k2d::vi2d(0 - scaled_ui.x * 2 - tile_size.x * 2,  tile_size.y * 0.5f + 1),
		k2d::vi2d(-scaled_ui.x * 0.5f, tile_size.y * 0.f-5),
		k2d::vi2d(scaled_ui.x * 2, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.15f, 26.0f, k2d::Color(255));
	pop_size_label->SetBackground(k2d::Color(129, 255));
	pop_size_label->SetVariable(&population_size);
	pop_size_label->SetModifiable(true);
	pop_size_label->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);

	ui_clickable_labels.push_back(pop_size_label);


	// top percentile label
	UIClickableLabel* top_percentile_label = new UIClickableLabel("TopPercentileClickable", "Top %: ",
		k2d::vi2d(0 - scaled_ui.x * 2, -scaled_ui.y * 0.5f - tile_size.y * 1 + 1),
		k2d::vi2d(-scaled_ui.x * 0.9f, tile_size.y * 0  - 5),
		k2d::vi2d(scaled_ui.x * 3 + tile_size.x, scaled_ui.y * 0.5f - 2),
		load_texture_from_cache("half"),
		sprite_batch, font1,
		0.12f, 26.0f, k2d::Color(255));
	top_percentile_label->SetBackground(k2d::Color(129, 255));
	top_percentile_label->SetVariable(&top_percentile);
	top_percentile_label->SetModifiable(true);
	top_percentile_label->SetPrettyPrintFunc(pretty_print_function_for_percents);
	top_percentile_label->SetPrintPrecision(0);
	top_percentile_label->AddCallbackFunction(this, &ConquestLocal::UpdateSelectionWeights);

	ui_clickable_labels.push_back(top_percentile_label);


	// Mutation rate label
	UIClickableLabel* mutation_label = new UIClickableLabel("MutationRateClickable", "R M. Rate: ",
		k2d::vi2d(0 - scaled_ui.x * 2, - scaled_ui.y * 1.0f - tile_size.y * 1+1),
		k2d::vi2d(-scaled_ui.x * 0.9f, tile_size.y * 0.f - 5),
		k2d::vi2d(scaled_ui.x * 3 + tile_size.x, scaled_ui.y * 0.5f - 2),
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
		k2d::vi2d(0 - scaled_ui.x * 2, -scaled_ui.y * 1.5f - tile_size.y * 1+1),
		k2d::vi2d(-scaled_ui.x * 0.9f, - 5),
		k2d::vi2d(scaled_ui.x * 3 + tile_size.x, scaled_ui.y * 0.5f - 2),
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
		k2d::vi2d(0 - scaled_ui.x * 2, -scaled_ui.y * 2.0f - tile_size.y * 1+1),
		k2d::vi2d(-scaled_ui.x * 0.9f, - 5),
		k2d::vi2d(scaled_ui.x * 3 + tile_size.x, scaled_ui.y * 0.5f - 2),
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

	// Create new map button
	UIElement* new_map_button = new UIElement("CreateNewMapButton", k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y - tile_size.y * 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255), load_texture_from_cache("full"), sprite_batch),
		create_text("New Map", 0.13f, 25.0f));
	new_map_button->SetIsButton(true);
	new_map_button->SetIsActive(true);
	new_map_button->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, 0));

	new_map_button->AddChild(new UIElement("CreateNewMapButtonDarkout", k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y - tile_size.y * 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 21.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0, 0, 0, 128), load_texture_from_cache("full"), sprite_batch),
		create_text("Queued", 0.13f, 25.0f)));
	new_map_button->GetChild()->SetIsActive(false);
	new_map_button->GetChild()->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.4f, -tile_size.y));

	ui_elements.push_back(new_map_button);


	// Pause button
	UIElement* pause_button = new UIElement("PauseButton", k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 + scaled_ui.y),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255), load_texture_from_cache("full"), sprite_batch),
		create_text("Pause", 0.15f, 25.0f));
	pause_button->SetIsActive(true);
	pause_button->SetIsButton(true);
	pause_button->SetTextOffset(k2d::vf2d(-tile_size.x * 1.3f, -tile_size.y * 0.2f));

	pause_button->AddChild(new UIElement("PauseButtonDarkout", k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 + scaled_ui.y),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 21.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0, 0, 0, 128), load_texture_from_cache("full"), sprite_batch),
		0));


	ui_elements.push_back(pause_button);


	// Player scoreboards
	UIElement* p1 = new UIElement("P1Score", k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
		glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(0), load_texture_from_cache("full"), sprite_batch),
		create_text("P1", 0.15f, 25.0f));
	p1->SetIsActive(false);
	p1->SetTextOffset(k2d::vf2d(-tile_size.x, 0));
	ui_elements.push_back(p1);

	UIElement* p2 = new UIElement("P2Score",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3) , tile_size.y * map_size.y - scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
		glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(1), load_texture_from_cache("full"), sprite_batch),
		create_text("P2", 0.15f, 25.0f));
	p2->SetIsActive(false);
	p2->SetTextOffset(k2d::vf2d(-tile_size.x, 0));
	ui_elements.push_back(p2);


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



	// Generation history graph
	generation_history = new UIGraph("GenHistory",
		k2d::vi2d(0 -tile_size.x * 0.5f + scaled_ui.x * 2 + tile_size.x * 2, -scaled_ui.y * 2 - tile_size.y * 2),
		k2d::vi2d(scaled_ui.x * 5, scaled_ui.y * 2), 100, 1200,
		load_texture_from_cache("full"), sprite_batch);
	generation_history->AddHorizontalLine(0.5f, k2d::Color(255, 0, 0, 128));
	generation_history->SetBackground(k2d::Color(40, 255));

	// Current gen tiles owned histogram
	current_gen_tiles_owned_histogram = new UIGraph("CurrentGenTilesOwned",
		k2d::vi2d(0 - tile_size.x * 0.5f + scaled_ui.x * 3 + scaled_ui.x * 4 + tile_size.x * 2, -scaled_ui.y * 2 - tile_size.y * 2),
		k2d::vi2d(scaled_ui.x * 5, scaled_ui.y * 2), 200, 1200,
		load_texture_from_cache("full"), sprite_batch);
	current_gen_tiles_owned_histogram->AddHorizontalLine(0.5f, k2d::Color(255, 0, 0, 128));
	current_gen_tiles_owned_histogram->SetBackground(k2d::Color(20, 255));

	// Current gen tiles owned histogram
	pick_chance_graph = new UIClickableGraph("PickChanceGraph",
		k2d::vi2d(0 - scaled_ui.x * 2, tile_size.y * map_size.y * 0.5f - scaled_ui.y * 0.5f - tile_size.y * 0.5f),
		k2d::vi2d(scaled_ui.x * 2.5f, scaled_ui.y * 2), ceil(population_size * top_percentile),
		find_max(0, ceil(population_size * top_percentile), pick_chance_function), // max_data_value
		load_texture_from_cache("full"), sprite_batch);
	pick_chance_graph->SetBackground(k2d::Color(20, 255));
	//pick_chance_graph->UpdateGraphValues();

	return 0;
}

int ConquestLocal::run()
{
	engine->RunExternalLoop(fps_target);
	main_loop();
	
	return 0;
}

// returns the winners id
int ConquestLocal::PlayGame(AI* first, AI* second)
{
	game_in_progress = true;

	SimpleAI* tmp1 = dynamic_cast<SimpleAI*>(first);
	if (tmp1)
	{
		tmp1->SetMapSize(server_sim.GetMapSize());
		tmp1->SetStartingPosition(server_sim.GetStartingPositions()[0]);
		tmp1->SetCurrentColorOwned(0);
	}

	SimpleAI* tmp2 = dynamic_cast<SimpleAI*>(second);
	if (tmp2)
	{
		tmp2->SetMapSize(server_sim.GetMapSize());
		tmp2->SetStartingPosition(server_sim.GetStartingPositions()[1]);
		tmp2->SetCurrentColorOwned(1);
	}

	server_sim.ConnectToServer(first->GetClientId());
	server_sim.ConnectToServer(second->GetClientId());

	// Set the first players id as the turn player id
	first->SetCurrentTurnPlayersId(first->GetClientId());
	second->SetCurrentTurnPlayersId(first->GetClientId());

	// Agents are in game
	first->SetInGame(true);
	second->SetInGame(true);

	// First goes first etc.
	first->SetWhichPlayerAmI(0);
	second->SetWhichPlayerAmI(1);


	server_sim.StartGame();

	return 0;
}

int ConquestLocal::main_loop()
{
	while (engine->GetRunning())
	{
		engine->ReadyRendering();

		if (!server_sim.GetGameInProgress())
		{
			if (last_played_index >= 0)
			{
				server_sim.DisconnectFromServer(ai_agents.at(last_played_index)->GetClientId());
				server_sim.DisconnectFromServer(opponent->GetClientId());
				CheckIfBestOfGeneration();
				CalculateGenerationAverage();
				SetPreviousIdAndTileCount();


				current_gen_tiles_owned_histogram->AddDataPoint(previous_tiles_owned);
			}

			last_played_index++;

			// One epoch done
			if (last_played_index >= ai_agents.size())
			{
				// 
				GeneticAlgorithm();
				last_played_index = 0;

				current_best_of_gen_id = 0;
				current_best_of_gen_tiles_owned = 0;

				generation_history->AddDataPoint(average_score_this_generation);
				average_score_this_generation = 0.0;
				// Create a new map after the generation has played their games
				if (should_create_new_map)
				{
					should_create_new_map = false;
					server_sim.CreateNewMap();
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
			server_sim.Update();

			for (AI* ai : ai_agents)
			{
				// Remove later TODO
				ai->Update();
			}

			opponent->Update();
		}
		
		tilemap = server_sim.GetBoardState();
		players = server_sim.GetPlayers();

		// TODO: remoev if
		if (ui_enabled)
		{
			UpdateTileColors();
			UpdateButtonColors();
			UpdateScoreboardColors();
			UpdateUIButtons();
			UpdateBarColors();
			UpdateGenerationsText();
		}

		ClampGeneticAlgorithmVariables();

		Event e = server_sim.GetNextEventFromQueue(spectator_id);

		HandleEvent(e);

		timer_counter += dt;

		// TODO: remoev if
		if (ui_enabled)
		{
			generation_history->Update(dt);
			current_gen_tiles_owned_histogram->Update(dt);
			pick_chance_graph->Update(dt);

			for (GameObject* tile : tiles)
			{
				// Draw tile
				tile->Update(dt);
			}

			// Update gameobjects
			for (UIElement* ui : ui_elements)
			{
				ui->Update(dt);
			}

			for (UIElement* ui : bar)
			{
				ui->Update(dt);
			}

			for (UIClickableLabel* l : ui_clickable_labels)
			{
				l->Update(dt);
			}
		}

		/*for (UIElement* b : buttons)
		{
			b->DestroyChildren();

			b->Update(dt);
		}*/

		update_input();
		//engine->SetWindowTitle("Conquest AI Training. fps: " + std::to_string(engine->GetCurrentFPS()));
		dt = engine->Update();
	}

	return 0;
}

void ConquestLocal::GeneticAlgorithm()
{
	// Sort best first
	std::sort(ai_agents.begin(), ai_agents.end(), [](AI* a, AI* b) -> bool
	{
		return a->GetTilesOwned() > b->GetTilesOwned();
	});

	int cutoff_index = ceil(top_percentile * population_size);
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
		//int index2 = distribution(random_engine);

		std::cout << "index = " << index1 << "\n";
		//int index1 = Random::get(0, max_index);
		//int index2 = Random::get(0, max_index);
		NeuralAI* tmp1 = dynamic_cast<NeuralAI*>(ai_agents[index1]);
		//NeuralAI* tmp2 = dynamic_cast<NeuralAI*>(ai_agents[index2]);
		//NeuralAI* child = new NeuralAI(tmp1, tmp2, running_agent_id++, &server_sim);

		NeuralAI* child = new NeuralAI(*tmp1, running_agent_id++, &server_sim);

		// Randomize the mutation type
		//if (index1 == index2)
		
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
	}

	k2d::KUSI_DEBUG("\n\n\n\n\n EPOCH %i DONE \n\n\n\n\n", epoch++);

	return;
}

void ConquestLocal::update_input()
{

	if (engine->GetInputManager().IsButtonPressedThisFrame(SDL_BUTTON_LEFT))
	{
		// Check which button is pressed.
		k2d::vi2d click_pos = engine->ScreenToWorld(engine->GetMouseCoords());
		std::cout << "click: " << click_pos << "\n";

		for (UIElement* button : ui_elements)
		{
			if (button->IsActive() && button->IsButton())
			{
				// BOt left position
				k2d::vi2d button_pos;
				button_pos.x = button->GetSprite()->GetPosition().x - button->GetSprite()->GetDimensions().x / 2;
				button_pos.y = button->GetSprite()->GetPosition().y - button->GetSprite()->GetDimensions().y / 2;
				k2d::vi2d button_dims;
				button_dims.x = button->GetSprite()->GetDimensions().x;
				button_dims.y = button->GetSprite()->GetDimensions().y;
				// Check if its a hit
				if (click_pos.x > button_pos.x && click_pos.x < (button_pos.x + button_dims.x)
					&& click_pos.y > button_pos.y && click_pos.y < (button_pos.y + button_dims.y))
				{
					std::cout << "HIT BUTTON " << button->GetName() << "!!\n";
					button->SetIsHit(true);
				}
			}
		}

		for (UIClickableLabel* l : ui_clickable_labels)
		{
			if (l->IsActive())
			{
				// BOt left position
				k2d::vi2d button_pos;
				button_pos.x = l->GetPosition().x - l->GetSize().x / 2;
				button_pos.y = l->GetPosition().y - l->GetSize().y / 2;
				k2d::vi2d button_dims;
				button_dims.x = l->GetSize().x;
				button_dims.y = l->GetSize().y;

				int dx = click_pos.x - button_pos.x;
				int dy = click_pos.y - button_pos.y;

				// Check if its a hit
				if (dx > 0 && dx < button_dims.x
					&& dy > 0  && dy < button_dims.y)
				{
					l->OnHit(k2d::vi2d(dx, dy));
				}
			}
		}

		UIClickableGraph* l = pick_chance_graph;
		if (l->IsActive())
		{
			// BOt left position
			k2d::vi2d button_pos;
			button_pos.x = l->GetPosition().x - l->GetSize().x / 2;
			button_pos.y = l->GetPosition().y;
			k2d::vi2d button_dims;
			button_dims.x = l->GetSize().x;
			button_dims.y = l->GetSize().y;

			int dx = click_pos.x - button_pos.x;
			int dy = click_pos.y - button_pos.y;

			// Check if its a hit
			if (dx > 0 && dx < button_dims.x
				&& dy > 0 && dy < button_dims.y)
			{
				l->OnHit(k2d::vi2d(dx, dy));
			}
		}

		//for (size_t i = 0; i < num_colors; i++)
		//{
		//	if (buttons.at(i)->IsActive())
		//	{
		//		// BOt left position
		//		k2d::vi2d button_pos;
		//		button_pos.x = buttons.at(i)->GetSprite()->GetPosition().x - buttons.at(i)->GetSprite()->GetDimensions().x / 2;
		//		button_pos.y = buttons.at(i)->GetSprite()->GetPosition().y - buttons.at(i)->GetSprite()->GetDimensions().y / 2;
		//		k2d::vi2d button_dims;
		//		button_dims.x = buttons.at(i)->GetSprite()->GetDimensions().x;
		//		button_dims.y = buttons.at(i)->GetSprite()->GetDimensions().y;
		//		// Check if its a hit
		//		if (click_pos.x > button_pos.x && click_pos.x < (button_pos.x + button_dims.x)
		//			&& click_pos.y > button_pos.y && click_pos.y < (button_pos.y + button_dims.y))
		//		{
		//			//std::cout << "hit: " << i << "\n";
		//			// If hit, break from loop
		//			break;
		//		}
		//	}
		//}
	}

	if (engine->GetInputManager().IsButtonPressed(SDL_BUTTON_RIGHT))
	{
		k2d::vf2d click_pos = engine->ScreenToWorld(engine->GetMouseCoords());

		UIClickableGraph* l = pick_chance_graph;
		if (l->IsActive())
		{
			// BOt left position
			k2d::vi2d button_pos;
			button_pos.x = l->GetPosition().x - l->GetSize().x / 2;
			button_pos.y = l->GetPosition().y;
			k2d::vi2d button_dims;
			button_dims.x = l->GetSize().x;
			button_dims.y = l->GetSize().y;

			int dx = click_pos.x - button_pos.x;
			int dy = click_pos.y - button_pos.y;

			// Check if its a hit
			if (dx > 0 && dx < button_dims.x
				&& dy > 0 && dy < button_dims.y)
			{

				l->OnHit(k2d::vi2d(dx, dy));
			}
		}
	}

	if (get_ui_by_name("OpponentChoice")->IsHit())
	{
		bad_ai_enabled = !bad_ai_enabled;

		get_ui_by_name("OpponentChoice")->SetIsHit(false);
	}

	if (get_ui_by_name("PauseButton")->IsHit())
	{
		paused = !paused;
		get_ui_by_name("PauseButton")->SetIsHit(false);
	}

	if (get_ui_by_name("CreateNewMapButton")->IsHit())
	{
		should_create_new_map = true;
		get_ui_by_name("CreateNewMapButton")->SetIsHit(false);
	}

	if (engine->GetInputManager().IsMouseWheelScrolledThisFrame(k2d::WheelDirection::UP))
	{
		variable_change_multiplier /= 10;
		variable_change_multiplier = k2d::clamp(variable_change_multiplier, 1, 1000000);
		for (UIClickableLabel* l : ui_clickable_labels)
		{
			l->SetVariableMultiplier(variable_change_multiplier);
		}
	}
	if (engine->GetInputManager().IsMouseWheelScrolledThisFrame(k2d::WheelDirection::DOWN))
	{
		variable_change_multiplier *= 10;
		variable_change_multiplier = k2d::clamp(variable_change_multiplier, 1, 1000000);
		for (UIClickableLabel* l : ui_clickable_labels)
		{
			l->SetVariableMultiplier(variable_change_multiplier);
		}
	}


	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_g))
	{
		bad_ai_enabled = true;
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_h))
	{
		bad_ai_enabled = false;
	}

	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_j))
	{
		should_create_new_map = true;
	}



	// Disable label rendering/updating
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_v))
	{
		for (UIClickableLabel* l : ui_clickable_labels)
		{
			l->SetIsActive(!l->IsActive());
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
		UpdateButtonColors();
		UpdateScoreboardColors();
		UpdateBarColors();
	}
}

void ConquestLocal::UpdateTileColors()
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

void ConquestLocal::UpdateButtonColors()
{
	////Clear UI elements, except mousecoords
	//for (int i = 0; i < buttons.size(); i++)
	//{
	//	delete buttons.at(i);
	//}
	//buttons.resize(0);


	//// Create UI Color Buttons for input
	//for (size_t i = 0; i < num_colors; i++)
	//{
	//	UIElement* button = new UIElement("Button",
	//		k2d::vi2d(-tile_size.x / 2 + tile_size.x * 2 + tile_size.x * 4 * i, -tile_size.y * 4 + tile_size.y / 2),
	//		new k2d::Sprite(glm::vec2(0.0f, 0.0f),
	//			tile_size.x * 4, tile_size.y * 4, 30.0f,
	//			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(i),
	//			load_texture_from_cache("full"), sprite_batch),
	//		0);
	//	// If the color is taken, hide the button
	//	/*if (taken_colors.at(i) == true)
	//	{
	//		button->SetIsActive(false);
	//	}*/
	//	
	//	
	//	buttons.push_back(button);
	//}
}

void ConquestLocal::UpdateUIButtons()
{

	if (bad_ai_enabled)
	{
		get_ui_by_name("OpponentChoice")->GetChild()->GetSprite()->SetPosition(glm::vec2(tile_size.x * map_size.x + scaled_ui.x - tile_size.x / 2, tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2));
	}
	else
	{
		get_ui_by_name("OpponentChoice")->GetChild()->GetSprite()->SetPosition(glm::vec2(tile_size.x * map_size.x + scaled_ui.x - (tile_size.x * 2.5f), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2));
	}

	if (paused)
	{
		get_ui_by_name("PauseButton")->GetChild()->SetIsActive(true);
	}
	else
	{
		get_ui_by_name("PauseButton")->GetChild()->SetIsActive(false);
	}

	if (should_create_new_map)
	{
		get_ui_by_name("CreateNewMapButton")->GetChild()->SetIsActive(true);
	}
	else
	{
		get_ui_by_name("CreateNewMapButton")->GetChild()->SetIsActive(false);
	}

}

void ConquestLocal::UpdateScoreboardColors()
{
	// Score "boards"
	// 8 = max players
	for (size_t i = 0; i < players.size(); i++)
	{
		if (players[i].id >= 0)
		{
			std::string scorep1 = std::to_string(players[i].tiles_owned);
			std::string ui_name = "P" + std::to_string(i + 1) + "Score";
			get_ui_by_name(ui_name)->GetSprite()->SetColor(skins.at(players[i].num_owned));
			//TODO: clean up this v
			get_ui_by_name(ui_name)->SetActualText(std::to_string(players[i].id));
			//get_ui_by_name(ui_name)->SetActualText(std::to_string(ai_agents[i]->GetGamesWon()));
			get_ui_by_name(ui_name)->SetIsActive(true);
		}
	}
}

void ConquestLocal::UpdateBarColors()
{
	float num_player_tiles = 0;
	float num_tiles = map_size.x * map_size.y;
	float num_remaining_tiles = num_tiles;
	for (int i = 0; i < players.size(); i++)
	{
		if (players[i].id >= 0 && players[i].id != players[0].id)
		{
			num_remaining_tiles -= players[i].tiles_owned;
		}
	}

	float start = -tile_size.x / 2;
	float map_width = tile_size.x * map_size.x;

	for (UIElement* ui : bar)
	{
		delete ui;
	}

	bar.clear();

	// player of this client always goes on the left
	float width = ceil((players[0].tiles_owned * map_width) / num_tiles);
	UIElement* bar_ui0 = new UIElement("Bar",
		k2d::vi2d(ceil(start + width / 2), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), width, tile_size.y * 2, 30.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(players[0].num_owned), load_texture_from_cache("full"), sprite_batch),
		0);
	start += width;
	bar.push_back(bar_ui0);
	num_remaining_tiles -= players[0].tiles_owned;

	width = ceil((num_remaining_tiles * map_width) / num_tiles);
	UIElement* darkout = new UIElement("Bar",
		k2d::vi2d(ceil(start + width / 2), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), width, tile_size.y * 2, 30.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(40, 40, 40, 255), load_texture_from_cache("full"), sprite_batch),
		0);
	start += width;
	bar.push_back(darkout);

	for (int i = 0; i < players.size(); i++)
	{
		if (players[i].id != players[0].id && players[i].id >= 0)
		{
			width = ceil((players[i].tiles_owned * map_width) / num_tiles);
			UIElement* bar_ui0 = new UIElement("Bar",
				k2d::vi2d(ceil(start + width / 2), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
				new k2d::Sprite(glm::vec2(0.0f, 0.0f), width, tile_size.y * 2, 30.0f,
					glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(players[i].num_owned), load_texture_from_cache("full"), sprite_batch),
				0);
			start += width;
			bar.push_back(bar_ui0);
			num_remaining_tiles -= players[i].tiles_owned;
		}
	}

	float offset = ceil(current_best_of_gen_tiles_owned * map_width / num_tiles);
	UIElement* current_best = new UIElement("CurrentBestBar",
		k2d::vi2d(ceil(-tile_size.x / 2 + offset), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), 1.0f, tile_size.y* 2, 31.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255), load_texture_from_cache("full"), sprite_batch),
		0);
	current_best->SetIsActive(true);
	bar.push_back(current_best);

	offset = ceil(map_width / 2);
	UIElement* halfway = new UIElement("HalfwayBar",
		k2d::vi2d(ceil(-tile_size.x / 2 + offset), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), 1.0f, tile_size.y * 2, 31.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255,255,255, 50), load_texture_from_cache("full"), sprite_batch),
		0);
	halfway->SetIsActive(true);
	bar.push_back(halfway);

	offset = ceil(average_score_this_generation * map_width / num_tiles);
	UIElement* average = new UIElement("AverageBar",
		k2d::vi2d(ceil(-tile_size.x / 2 + offset), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), 1.0f, tile_size.y * 2, 31.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255, 255, 255, 150), load_texture_from_cache("full"), sprite_batch),
		0);
	average->SetIsActive(true);
	bar.push_back(average);
}

void ConquestLocal::UpdateGenerationsText()
{
	std::string genetext = "Generation: " + std::to_string(epoch)+ ", avg F: " + std::to_string((int) average_score_this_generation);
	std::string ui_name = "Generation";
	get_ui_by_name(ui_name)->SetActualText(genetext);

	std::string best_text = "Gen Best ID: " + std::to_string(current_best_of_gen_id) + ", F: " + std::to_string(current_best_of_gen_tiles_owned);
	get_ui_by_name(ui_name)->GetChild()->SetActualText(best_text);

	std::string prev_text = "Previous ID: " + std::to_string(previous_id) + ", F: " + std::to_string(previous_tiles_owned);
	get_ui_by_name("PreviousText")->SetActualText(prev_text);
}

void ConquestLocal::CalculateNewSelectionWeights()
{
	selection_weights.resize(ceil(population_size * top_percentile));
	for (size_t i = 0; i < selection_weights.size(); i++)
	{
		// Good looking function
		selection_weights.at(i) = pick_chance_function(i);
	}
}


void ConquestLocal::UpdateSelectionWeights()
{
	// Initialize the weights used for the selection of agents
	//selection_weights.clear();
	selection_weights.resize(ceil(population_size * top_percentile));
	//for (size_t i = 0; i < population_size; i++)
	//{
	//	// Good looking function
	//	//selection_weights.push_back(pick_chance_function(i));

	//}
	pick_chance_graph->SetMaxDataValue(find_max(0, ceil(population_size * top_percentile), pick_chance_function));
	pick_chance_graph->SetDataToFollow(&selection_weights);
}


void ConquestLocal::ClampGeneticAlgorithmVariables()
{
	top_percentile = k2d::clamp(top_percentile, 0.01f, 1.0f);

	mutation_rate = k2d::clamp(mutation_rate, 0.000001f, 1.0f);
	close_mutation_rate = k2d::clamp(close_mutation_rate, 0.000001f, 1.0f);
	close_mutation_epsilon = k2d::clamp(close_mutation_epsilon, 0.000001, 1.0);

	mutation_type_chance = k2d::clamp(mutation_type_chance, 0.0f, 1.0f);

	population_size = k2d::clamp(population_size, 1, 100000);
}

void ConquestLocal::CalculateGenerationAverage()
{
	double sum = 0.0;
	for (size_t i = 0; i < ai_agents.size(); i++)
	{
		sum += ai_agents[i]->GetTilesOwned();
	}

	average_score_this_generation = sum / ((double) last_played_index + 1.0);
}

void ConquestLocal::CheckIfBestOfGeneration()
{
	if (ai_agents.at(last_played_index)->GetTilesOwned() > current_best_of_gen_tiles_owned)
	{
		current_best_of_gen_id = ai_agents.at(last_played_index)->GetClientId();
		current_best_of_gen_tiles_owned = ai_agents.at(last_played_index)->GetTilesOwned();
	}
}

void ConquestLocal::SetPreviousIdAndTileCount()
{
	previous_id = ai_agents[last_played_index]->GetClientId();
	previous_tiles_owned = ai_agents[last_played_index]->GetTilesOwned();
}

void ConquestLocal::GetRandomColorFromLoadedSkins(int index)
{
	std::uniform_int_distribution<int> rand_col(0, loaded_skins.size() - 1);

	skins.at(index) = loaded_skins.at(rand_col(random_engine));

	UpdateTileColors();
	UpdateButtonColors();
	UpdateScoreboardColors();
	UpdateBarColors();
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

int ConquestLocal::load_texture_into_cache(const char* friendly_name, std::string filename)
{
	// Lookup texturemap
	auto mit = m_texture_cache.find(friendly_name);

	// If its not in the map
	if (mit == m_texture_cache.end())
	{
		k2d::GLTexture tex = k2d::ImageLoader::LoadPNG(filename, false);

		// Insert it into the map
		m_texture_cache.insert(std::make_pair(friendly_name, tex));
	}

	return 0;
}

k2d::GLTexture ConquestLocal::load_texture_from_cache(const char* friendly_name)
{
	// Lookup texturemap
	auto mit = m_texture_cache.find(friendly_name);

	// If its not in the map
	if (mit == m_texture_cache.end())
	{
		k2d::KUSI_DEBUG("Cannot find texture from cache, name: %s\nCreating new default texture", friendly_name);
		// Make new texture from default image
		return k2d::ImageLoader::LoadPNG("Textures/default.png", false);
	}

	return mit->second;
}

k2d::Sprite* ConquestLocal::create_tile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), tile_size.x, tile_size.y, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}

k2d::Sprite* ConquestLocal::create_projectile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), 8, 8, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}


k2d::Text* ConquestLocal::create_text(std::string text, float scale, float depth)
{
	return new k2d::Text(text, font1, 0, 0, scale, depth, k2d::Color(255), sprite_batch);
}

UIElement* ConquestLocal::get_ui_by_name(std::string name)
{
	for (UIElement* ui : ui_elements)
	{
		if (name == ui->GetName())
		{
			return ui;
		}
	}
	return nullptr;
}

/**
	 *	Loads a font from the texture cache.\n
	 *	If the font is not in the cache, \n
	 *	it gets loaded into it from the specified file
	 */
std::map<GLchar, k2d::Character> ConquestLocal::LoadFont(const char* _file)
{
	if (FT_Init_FreeType(&ft))
	{
		k2d::KUSI_ERROR("ERROR::FREETYPE: Could not init Freetype Library");
	}

	if (FT_New_Face(ft, _file, 0, &face))
	{
		k2d::KUSI_ERROR("ERROR::FREETYPE: Failed to load font");
	}

	FT_Set_Pixel_Sizes(face, 0, 128);

	// Lookup texturemap
	auto mit = font_cache.find(_file);


	// If its not in the map
	if (mit == font_cache.end())
	{
		std::map<GLchar, k2d::Character> new_map = LoadChars();

		// Insert it into the map
		font_cache.insert(std::make_pair(_file, new_map));
		return new_map;
	}

	// return texture
	return mit->second;
}

/**
 *	Loads characters of a font into a usable format
 */
std::map<GLchar, k2d::Character> ConquestLocal::LoadChars()
{
	std::map<GLchar, k2d::Character> characters = {};

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Try for the first 128 chars
	for (GLubyte c = 0; c < 128; c++)
	{
		// Try to load glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			k2d::KUSI_DEBUG("ERROR::FREETYPE: Failed to load Glyph");
			continue;
		}

		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_ALPHA,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_ALPHA,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		k2d::Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		characters.insert(std::pair<GLchar, k2d::Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// Free memory
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return characters;
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

		taken_colors = server_sim.GetTakenColors();
		turns_played = server_sim.GetTurnsPlayed();

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