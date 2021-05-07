#include <core/ConquestMultiplayer.h>

const int packet_type_input_data = 1;
const int packet_type_tile_data = 2;
const int packet_type_command = 3;
const int packet_type_invalid_color = 4;
const int packet_type_turn_change = 5;
const int packet_type_game_end = 6;

ConquestMultiplayer::ConquestMultiplayer(std::string title, int width, int height, int target_fps, bool v_sync) :
	k2d::Application(title, width, height, target_fps, v_sync)
{
	Setup();
}

ConquestMultiplayer::~ConquestMultiplayer()
{
}

void ConquestMultiplayer::Setup()
{
	// Basics
	SetShaders("Shaders/core.vert", "Shaders/core.frag", "core", { "vertex_position", "vertex_color", "vertex_uv" });
	engine->SetCameraPosition(k2d::vi2d(110, 230));
	font1 = LoadFont("Fonts/opensans.ttf");

	// Session id for the database table. time from 1-1-2000
	time_t t;
	struct tm y2k = { 0 };
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
	time(&t);
	int ses_id = difftime(t, mktime(&y2k));

	// Init game
	init_game();
	create_ai();

	create_ui();

	// Start the main loop by calling base::Setup()
	k2d::Application::Setup();
}

void ConquestMultiplayer::TryToConnectToServer()
{
	should_send_ready = true;
	SendCommandToServer(80, should_send_ready);
	//get_ui_by_name("StartButton")->SetActive(false);
}

void ConquestMultiplayer::StartGame()
{
	generate_input_buttons();
	get_ui_by_name("StartButton")->SetActive(false);
	nn_display->SetNeuralNetPtr(neural_ai->GetNeuralNet());
	should_send_ready = true;
	SendCommandToServer(80, should_send_ready);
	RevealUI();
	//get_ui_by_name("StartButton")->SetActive(false);
}

void ConquestMultiplayer::ReadyUpForNewGame()
{
	should_send_reset = true;
}

float ConquestMultiplayer::weight_selection_function(float x, float a, float b)
{
	return a / (x + b);
}

int ConquestMultiplayer::init_game()
{
	// Init with mapsize, colors
	load_texture_into_cache("empty", "Textures/tiles/square100x100.png");
	load_texture_into_cache("selected", "Textures/tiles/selection100x100.png");
	load_texture_into_cache("ss", "Textures/tiles/ss100x100.png");
	load_texture_into_cache("dot", "Textures/tiles/dot100x100.png");
	load_texture_into_cache("full", "Textures/tiles/full100x100.png");
	load_texture_into_cache("full_i", "Textures/tiles/full100x100.png", true);
	load_texture_into_cache("half", "Textures/tiles/halfalpha100x100.png");



	load_texture_into_cache("ui", "Textures/ui/ui.png");
	load_texture_into_cache("unitcard", "Textures/ui/ui100x300.png");

	std::ifstream netfile("config/net.txt");
	if (!netfile) {
		k2d::KUSI_ERROR("SKIN LOADING ERROR Q");
	}
	int a;
	int b;
	int c;
	int d;
	int port;
	netfile >> a >> b >> c >> d >> port;
	SERVER_ADDRESS = MAKEADDRESS(a, b, c, d); //localhost
	SERVER_PORT = port;

	netfile >> a >> b >> c >> d >> port;
	address = MAKEADDRESS(a, b, c, d);
	netfile.close();

	// Send packets every 33ms = 30hz
	send_packets_every_ms = 0.033f;
	timer_counter = 0.0f;
	map_size = { 0,0 };

	player_id = Random::get(100, 100000);

	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = htonl(address);
	socket_address.sin_port = htons(0);

	// set socket non blocking
	result = ioctlsocket(sock, FIONBIO, &NON_BLOCKING);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(SERVER_ADDRESS);
	server_address.sin_port = htons(SERVER_PORT);

	address_length = (int32_t)sizeof(struct sockaddr_in);

	input_num = -1;
	should_send_ready = false;
	should_send_reset = false;
	should_send_drop_all = false;
	scoreboard_init = false;

	this_player_index = 0;

	blink_timer = 0.f;
	blink_every_second = 1.0f;

	header_size = sizeof(packet_header_t);

	tile_size.x = 20;
	tile_size.y = 20;

	std::ifstream videofile("config/video.txt");
	if (!videofile) {
		k2d::KUSI_ERROR("VIDEO CONFIG LOADING ERRO");
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
	map_size = {0, 0};
	num_colors = 0;

	game_state = GameState::NOTCONNECTED;

	connected_to_server = false;

	frame_buffer = 2;
	frame_buffer_counter = frame_buffer;

	return 0;
}



int ConquestMultiplayer::create_ai()
{
	simple_ai = new SimpleAI(&tilemap, &taken_colors);
	neural_ai = new NeuralAI("Data/net.neural", &tilemap, &taken_colors);

	current_ai = neural_ai;



	return 0;
}

void ConquestMultiplayer::generate_input_buttons()
{
	for (size_t i = 0; i < 10; i++)
	{
		UIButton* input_button = new UIButton("InputButton" + i,
			k2d::vi2d(i * scaled_ui.x + scaled_ui.x /2 - tile_size.x/2, -scaled_ui.y),
			k2d::vi2d(scaled_ui.x, scaled_ui.y),
			25.0f,
			CreateDefaultSprite("full", skins.at(i), 45.0f),
			nullptr);
		input_button->SetActive(true);
		ui_input_buttons.push_back(input_button);
	}
}

int ConquestMultiplayer::HandleAI()
{
	return current_ai->Update();
}

int ConquestMultiplayer::create_ui()
{
	scaled_ui = tile_size * 4;
	tile_brightness = 0.75f;

	variable_change_multiplier = 1;

	ui_enabled = true;

#pragma region MultiLabels
	/*
		MULTILINE LABELS
	*/

	// Winner card
	UIMultiLabel* winner_card = new UIMultiLabel("WinnerCard",
		k2d::vi2d(scaled_ui.x * -3, scaled_ui.y * -0.5f),
		k2d::vi2d(scaled_ui.x * 3, scaled_ui.y * 2),
		tile_size.y,
		0.15f,
		30.0f,
		font1,
		load_texture_from_cache("full"),
		sprite_batch);
	winner_card->AddBackground(k2d::Color(255, 128));
	winner_card->AddLabel("WinnerID", "ID: ", &winner_id);
	winner_card->AddLabel("WinnerScore", "Score: ", &winner_tiles);
	winner_card->SetActive(false);

	ui_multilabels.push_back(winner_card);

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
	p0_scoreboard->AddLabel("P0Scoreboard", "", &p0_tiles);
	p0_scoreboard->SetActive(false);


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
	p1_scoreboard->AddLabel("P1Scoreboard", "", &p1_tiles);
	p1_scoreboard->SetActive(false);

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
	tile_brightness_label->SetActive(false);
	tile_brightness_label->SetBackground(k2d::Color(129, 255));
	tile_brightness_label->SetVariable(&tile_brightness);
	tile_brightness_label->SetModifiable(true);
	tile_brightness_label->SetBaseMultiplier(0.05f);
	tile_brightness_label->SetPrintPrecision(2);
	tile_brightness_label->AddCallbackFunction(this, &ConquestMultiplayer::ClampTileBrightness);
	tile_brightness_label->AddCallbackFunction(this, &ConquestMultiplayer::UpdateTileBrightness);

	ui_clickable_labels.push_back(tile_brightness_label);

	/// The grid:
	/// x = 3 2 1
	/// -scaled_ui.x * grid_x * 2.5 + tile_size.x * grid_x * 1 + 1.5


	// Turns played text
	UIClickableLabel* turns_text = new UIClickableLabel("NRTurnsLabel", "Turns played: ",
		k2d::vi2d(0, tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		k2d::vf2d(0, -tile_size.y * 0.2f),
		k2d::vi2d(0, 0),
		load_texture_from_cache("empty"), sprite_batch, font1,
		0.15f, 35.0f, k2d::Color(255));
	turns_text->SetActive(false);
	turns_text->SetModifiable(false);
	turns_text->SetVariable(&turns_played);

	ui_clickable_labels.push_back(turns_text);


#pragma endregion Clickable labels


#pragma region Buttons
	/*
		BUTTONS
	*/
	// Opponent toggle button
	UIToggleButton* ai_choice = new UIToggleButton("AIChoice",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2),
		k2d::vi2d(scaled_ui.x, scaled_ui.y * 0.5f),
		k2d::vi2d(scaled_ui.x / 2, 0),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Neural   Bad ", 0.10f, 25.0f),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f),
			scaled_ui.x * 0.5f, scaled_ui.y * 0.5f,
			26.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0, 128), load_texture_from_cache("full"), sprite_batch));
	ai_choice->SetActive(false);
	ai_choice->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, -4));
	ai_choice->AddCallbackFunction(ai_choice, &UIToggleButton::ToggleFuncSideways);
	ai_choice->AddCallbackFunction(this, &ConquestMultiplayer::ToggleAIType);
	// Ugly position init
	ai_choice->GetDarkoutSprite()->SetPosition(glm::vec2(ai_choice->GetDarkoutSprite()->GetPosition().x + ai_choice->GetSize().x / 2, ai_choice->GetDarkoutSprite()->GetPosition().y));
	ai_choice->SetDarkoutActive(true);

	ui_buttons.push_back(ai_choice);

	UIButton* start_button = new UIButton("StartButton",
		k2d::vi2d(0, 0),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Connect", 0.12f, 25.0f));
	start_button->SetActive(true);
	start_button->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, 0));
	start_button->AddCallbackFunction(this, &ConquestMultiplayer::TryToConnectToServer);
	ui_buttons.push_back(start_button);

	UIButton* ready_up_for_new_game_button = new UIButton("ReadyForNewGameButton",
		k2d::vi2d(0, 0),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("Ready!", 0.12f, 25.0f));
	ready_up_for_new_game_button->SetActive(true);
	ready_up_for_new_game_button->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, 0));
	ready_up_for_new_game_button->AddCallbackFunction(this, &ConquestMultiplayer::ReadyUpForNewGame);
	ui_buttons.push_back(ready_up_for_new_game_button);

	UIToggleButton* ai_enable_button = new UIToggleButton("AIEnableButton",
		k2d::vi2d(0, 0),
		k2d::vi2d(scaled_ui.x, scaled_ui.y),
		k2d::vi2d(0, 0),
		25.0f,
		CreateDefaultSprite("full", k2d::Color(255, 255), 25.0f),
		create_text("AI Enable", 0.12f, 25.0f),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f),
			scaled_ui.x * 0.5f, scaled_ui.y * 0.5f,
			26.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0, 128), load_texture_from_cache("full"), sprite_batch));
	ai_enable_button->SetActive(false);
	ai_enable_button->SetTextOffset(k2d::vf2d(-scaled_ui.x * 0.5f, 0));
	ai_enable_button->SetDarkoutActive(false);
	ai_enable_button->AddCallbackFunction(ai_enable_button, &UIToggleButton::ToggleFuncOnOff);
	ai_enable_button->AddCallbackFunction(this, &ConquestMultiplayer::ToggleAIEnable);
	ui_buttons.push_back(ai_enable_button);

#pragma endregion buttons


#pragma region ScoreBar

	scorebar = new UIScoreBar("ScoreBar", k2d::vf2d(-tile_size.x *0.5f + tile_size.x * map_size.x * 0.5f, tile_size.y * map_size.y + tile_size.y * 2.5f),
		k2d::vf2d(tile_size.x * map_size.x, tile_size.y * 2.0f),
		25.0f,
		load_texture_from_cache("full"),
		sprite_batch);
	scorebar->AddBackground(k2d::Color(64, 255));
	scorebar->SetMaxTileCount(map_size.x * map_size.y);
	scorebar->SetVariablePointers(&p0_tiles, &p0_color, &p1_tiles, &p1_color);
	scorebar->AddMarker(&halfway, k2d::Color(255));
	scorebar->SetActive(false);

#pragma endregion


#pragma region Rectangles

	nn_vision_rect = new UIRectangle("VisionRect", 0, 0, 50.0f, CreateDefaultSprite("full", k2d::Color(255, 255, 0, 128)));
	nn_vision_rect->SetActive(false);

#pragma endregion

#pragma region NetDisplay

	nn_display = new UINetDisplay("NetDisplay", 
		k2d::vi2d(0 - scaled_ui.x * 7.0 + tile_size.x * 3.5, +scaled_ui.y * 4.5f - tile_size.y * 1.5),
		k2d::vi2d(scaled_ui.x * 4.5, scaled_ui.y * 8.0f + tile_size.y * 2),
		25.0f,
		load_texture_from_cache("full"),
		sprite_batch, this);
	nn_display->AddCallbackFunction(nn_display, &UINetDisplay::ToggleWeightsOnlyMode);
	nn_display->SetActive(false);
	//nn_display->AddBackground(k2d::Color(255, 128, 255, 128));

#pragma endregion

	// Insert all the freshly created ui elements into this vector
	all_of_the_ui.insert(all_of_the_ui.end(), ui_buttons.begin(), ui_buttons.end());
	all_of_the_ui.insert(all_of_the_ui.end(), ui_clickable_labels.begin(), ui_clickable_labels.end());
	all_of_the_ui.insert(all_of_the_ui.end(), ui_multilabels.begin(), ui_multilabels.end());
	all_of_the_ui.insert(all_of_the_ui.end(), ui_progressbars.begin(), ui_progressbars.end());
	all_of_the_ui.push_back(scorebar);
	all_of_the_ui.push_back(nn_vision_rect);
	all_of_the_ui.push_back(nn_display);

	return 0;
}

void ConquestMultiplayer::PreRender()
{
	k2d::Application::PreRender();
}

void ConquestMultiplayer::Update()
{
	timer_counter += dt;
	// NET CODE:
	// check if input needs to be sent
	if (timer_counter > send_packets_every_ms) {
		timer_counter = 0;
		if (input_num != -1) {
			int offset = 0;
			// Write the header
			packet_header_t h;
			h.id = player_id;
			h.packet_type = packet_type_input_data;
			write(buffer, &h, sizeof(h), &offset);

			// Write the data
			write(buffer, &input_num, sizeof(input_num), &offset);

			//std::cout << "INPUTNUM SENT: " << input_num << "\n";

			int result = sendto(sock, buffer, offset, 0, (struct sockaddr*)&server_address, address_length);
			input_num = -1;
		}

		if (should_send_ready)
		{
			// ready = code 80
			game_over = false;
			SendCommandToServer(80, should_send_ready);
		}
		if (should_send_reset)
		{
			// reset = code 100
			game_over = false;
			SendCommandToServer(100, should_send_reset);
		}
		if (should_send_drop_all)
		{
			// Drop all = code 89
			SendCommandToServer(89, should_send_drop_all);
		}
	}

	if (recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender_address, &address_length) >= header_size)
	{
		int offset = 0;
		packet_header_t h;

		read(buffer, &h, sizeof(h), &offset);

		switch (h.packet_type)
		{
		case packet_type_tile_data:
		{
			// read whose turn it is
			read(buffer, &turn_id, sizeof(int), &offset);
			read(buffer, &turns_played, sizeof(int), &offset);

			read(buffer, &num_players, sizeof(num_players), &offset);
			read(buffer, &players, sizeof(player_t) * num_players, &offset);
			read(buffer, &map_size.x, sizeof(uint8_t), &offset);
			read(buffer, &map_size.y, sizeof(uint8_t), &offset);

			// Read how many colors there are
			read(buffer, &num_colors, sizeof(uint8_t), &offset);
			taken_colors.resize(num_colors);

			// Read taken colors and push to vector
			for (size_t i = 0; i < num_colors; i++)
			{
				uint8_t temp;
				read(buffer, &temp, sizeof(uint8_t), &offset);
				if (temp == 1)
				{
					taken_colors.at(i) = true;
				}
				else
				{
					taken_colors.at(i) = false;
				}
				//std::cout << "Taken colors " << i << ": " << taken_colors.at(i)<< "\n";
			}

			tilemap.clear();

			// Populate the tilemap
			for (uint8_t y = 0; y < map_size.y; y++)
			{
				tilemap.push_back(std::vector<tile>());
				for (uint8_t x = 0; x < map_size.x; x++)
				{
					
					/*uint8_t new_tile;
					read(buffer, &new_tile, sizeof(uint8_t), &offset);

					tilemap[y][x].color = (new_tile & 0xf0) >> 4;
					tilemap[y][x].owner = (new_tile & 0x0f);*/
					tile new_tile;
					read(buffer, &new_tile, sizeof(tile), &offset);
					tilemap.at(y).push_back(new_tile);
				}
			}

			// 8 = max players
			for (size_t i = 0; i < 8; i++)
			{
				if (players[i].id != 0)
				{
					// Set scores for players

				}
			}

			int board_width = tile_size.x * map_size.x - 2 * tile_size.x;
			int board_height = -2 * tile_size.y + tile_size.y * map_size.y;

			//engine->SetCameraPosition((k2d::vf2d(board_width / 2, board_height / 2)));

			if (!connected_to_server)
			{
				StartGame();
				connected_to_server = true;
			}
			halfway = (map_size.x * map_size.y) / 2;
			sc.resize(8);
			sc[0] = { 0,0 };
			sc[1] = { map_size.x - 1, map_size.y - 1 };
			sc[2] = { 0, map_size.y - 1 };
			sc[3] = { map_size.x - 1 ,0 };
			sc[4] = { 0, map_size.y / 2 - 1 };
			sc[5] = { map_size.x - 1, map_size.y / 2 - 1 };
			sc[6] = { map_size.x / 2 - 1, 0 };
			sc[7] = { map_size.x / 2 - 1, map_size.y - 1 };

			for (size_t i = 0; i < num_players; i++)
			{
				if (players[i].id == player_id)
				{
					current_ai->SetWhichPlayerAmI(i);
					current_ai->SetStartingPosition(sc[i]);
					current_ai->SetCurrentColorOwned(i);
					current_ai->SetMapSize(k2d::vi2d(map_size.x, map_size.y));
				}
			}

			if (game_over == false)
			{
				game_state = GameState::INGAME;
			}

			break;
		}
		case packet_type_invalid_color:
		{
			int invalid_color = 0;

			// read whose turn it is
			read(buffer, &invalid_color, sizeof(invalid_color), &offset);

			try_to_play_best++;

			break;
		}
		case packet_type_turn_change:
		{
			// read whose turn it is
			read(buffer, &turn_id, sizeof(turn_id), &offset);

			num_of_each_color.clear();
			try_to_play_best = 0;


			break;
		}
		case packet_type_game_end:
		{
			int winner_id = 0;
			int turns_played = 0;
			read(buffer, &winner_id, sizeof(winner_id), &offset);
			read(buffer, &turns_played, sizeof(turns_played), &offset);

			this->winner_id = winner_id;

			// Loop through players and search the one with winner id
			for (size_t i = 0; i < 8; i++)
			{
				if (players[i].id == winner_id)
				{
					winner_index = i;
					this->winner_tiles = players[i].tiles_owned;
					break;
				}
			}

			game_state = GameState::GAME_OVER;
			game_over = true;
			frame_buffer_counter = frame_buffer;
			break;
		}
		default:
			break;
		}
	}

	if (ui_enabled)
	{
		if (game_state == GameState::INGAME || frame_buffer_counter > 0)
		{
			UnDisplayWinner();
			UpdateUIElementPositions();
			UpdateTileColors();
			UpdateScoreboardIds();
			UpdateScorebarValues();
			UpdateValidInputButtons();
			if (current_ai == neural_ai && AI_CONTROL)
			{
				UpdateDebugRectanglePosition();
			}
			else
			{
				get_ui_by_name("VisionRect")->SetActive(false);
				get_ui_by_name("NetDisplay")->SetActive(false);
			}

			if (turn_id == player_id && AI_CONTROL && game_state == GameState::INGAME)
			{
				input_num = HandleAI();
			}
			frame_buffer_counter--;
		}
		else if (game_state == GameState::GAME_OVER)
		{
			DisplayWinner();
		}
	}

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

		for (UIBase* b : ui_input_buttons)
		{
			b->Update(dt);
		}
	}

	update_input();
	//engine->SetWindowTitle("Conquest AI Training. fps: " + std::to_string(engine->GetCurrentFPS()));

	k2d::Application::Update();
}

void ConquestMultiplayer::CleanUp()
{
	int offset = 0;
	uint8_t code = 90;

	// Write the header
	packet_header_t h;
	h.id = player_id;
	h.packet_type = packet_type_command;
	write(buffer, &h, sizeof(h), &offset);

	// Write the data
	write(buffer, &code, sizeof(code), &offset);

	sendto(sock, buffer, offset, 0, (struct sockaddr*)&server_address, address_length);

	closesocket(sock);
	WSACleanup();
}

void ConquestMultiplayer::update_input()
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

		for (int i = 0; i < ui_input_buttons.size(); i++) 
		{
			if (ui_input_buttons.at(i)->IsActive())
			{
				k2d::vf2d button_pos;
				button_pos.x = ui_input_buttons[i]->GetPosition().x - ui_input_buttons[i]->GetSize().x / 2;
				button_pos.y = ui_input_buttons[i]->GetPosition().y - ui_input_buttons[i]->GetSize().y / 2;
				k2d::vf2d button_dims;
				button_dims.x = ui_input_buttons[i]->GetSize().x;
				button_dims.y = ui_input_buttons[i]->GetSize().y;

				int dx = click_pos.x - button_pos.x;
				int dy = click_pos.y - button_pos.y;
				// Check if its a hit
				if (dx > 0 && dx < button_dims.x
					&& dy > 0 && dy < button_dims.y)
				{
					input_num = i;
				}
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



	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_d))
	{
		should_send_drop_all = true;
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_r))
	{
		should_send_ready = true;
	}
	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_o))
	{
		should_send_reset = true;
	}

	if (engine->GetInputManager().IsKeyPressedThisFrame(SDLK_a))
	{
		AI_CONTROL = !AI_CONTROL;
	}

#pragma endregion
}

void ConquestMultiplayer::UpdateTileColors()
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
					create_tile_sprite("full", skins.at(tilemap[y][x].color), 35.0f)));
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
	for (size_t i = 0; i < ui_input_buttons.size(); i++)
	{
		ui_input_buttons.at(i)->GetSprite()->SetColor(skins.at(i));
	}
}

void ConquestMultiplayer::UpdateScoreboardIds()
{

	for (size_t i = 0; i < MAX_PLAYERS; i++)
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

void ConquestMultiplayer::UpdateScorebarValues()
{
	p0_tiles = players[0].tiles_owned;
	p0_color = k2d::Color(skins.at(players[0].num_owned));
	p1_tiles = players[1].tiles_owned;
	p1_color = k2d::Color(skins.at(players[1].num_owned));
	UIScoreBar* ui = static_cast<UIScoreBar*>(get_ui_by_name("ScoreBar"));
	if (ui)
	{
		ui->UpdateBar();
		ui->SetSize(k2d::vi2d(tile_size.x * map_size.x, tile_size.y * 2));
	}
}

void ConquestMultiplayer::ToggleAIType()
{
	if (current_ai == simple_ai)
	{
		current_ai = neural_ai;
	}
	else
	{
		current_ai = simple_ai;
	}
}

void ConquestMultiplayer::ClampTileBrightness()
{
	k2d::clamp(tile_brightness, 0.0f, 1.0f);
}

void ConquestMultiplayer::UpdateValidInputButtons()
{
	for (size_t i = 0; i < ui_input_buttons.size(); i++)
	{
		// Check if we have the color at all and check if its taken, if its taken, don't show it
		if (i < taken_colors.size() && taken_colors.at(i) != true)
		{
			ui_input_buttons.at(i)->SetActive(true);
		}
		else
		{
			ui_input_buttons.at(i)->SetActive(false);
		}
	}
}

void ConquestMultiplayer::UpdateUIElementPositions()
{
	scaled_ui = 4 * tile_size;
	get_ui_by_name("P0Scoreboard")->SetPosition(k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 - tile_size.y / 2));
	get_ui_by_name("P1Scoreboard")->SetPosition(k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y / 2 - tile_size.y / 2));

	get_ui_by_name("TileBrightnessLabel")->SetPosition(k2d::vi2d(0 + map_size.x * tile_size.x + scaled_ui.x * 0.5f + tile_size.x * 0.5f, map_size.y * tile_size.y + tile_size.y * 3.0f));

	get_ui_by_name("NRTurnsLabel")->SetPosition(k2d::vi2d(0, tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2));
	get_ui_by_name("AIChoice")->SetPosition(k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y / 2 - tile_size.y / 2));
	//get_ui_by_name("StartButton")->SetPosition(k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y - tile_size.y * 2.5f));
	get_ui_by_name("AIEnableButton")->SetPosition(k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y - scaled_ui.y - tile_size.y * 2.5f));
	get_ui_by_name("ReadyForNewGameButton")->SetPosition(k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * map_size.y - scaled_ui.y*2.25 - scaled_ui.y - tile_size.y * 2.5f));
	get_ui_by_name("ScoreBar")->SetPosition(k2d::vf2d(-tile_size.x * 0.5f + tile_size.x * map_size.x * 0.5f, tile_size.y * map_size.y + tile_size.y * 2.5f));
	get_ui_by_name("NetDisplay")->SetPosition(k2d::vi2d(0 - scaled_ui.x * 7.0 + tile_size.x * 3.5, +scaled_ui.y * 4.5f - tile_size.y * 1.5));

	scorebar->SetMaxTileCount(map_size.x * map_size.y);
}

void ConquestMultiplayer::UpdateTileBrightness()
{
	for (size_t i = 0; i < 10; i++)
	{
		skins.at(i).a = floor((float)255 * tile_brightness);
	}
}

void ConquestMultiplayer::RevealUI()
{
	get_ui_by_name("P0Scoreboard")->SetActive(true);
	get_ui_by_name("P1Scoreboard")->SetActive(true);

	get_ui_by_name("TileBrightnessLabel")->SetActive(true);

	get_ui_by_name("NRTurnsLabel")->SetActive(true);
	get_ui_by_name("AIChoice")->SetActive(true);
	
	//get_ui_by_name("StartButton")->SetActive(true);
	get_ui_by_name("AIEnableButton")->SetActive(true);
	get_ui_by_name("ScoreBar")->SetActive(true);
	get_ui_by_name("NetDisplay")->SetActive(true);
}

void ConquestMultiplayer::ToggleAIEnable()
{
	AI_CONTROL = !AI_CONTROL;
}

void ConquestMultiplayer::DisplayWinner()
{
	get_ui_by_name("WinnerCard")->SetActive(true);
	get_ui_by_name("ReadyForNewGameButton")->SetActive(true);
}

void ConquestMultiplayer::UnDisplayWinner()
{
	get_ui_by_name("WinnerCard")->SetActive(false);
	get_ui_by_name("ReadyForNewGameButton")->SetActive(false);
}

void ConquestMultiplayer::UpdateDebugRectanglePosition()
{
	UIRectangle* l = static_cast<UIRectangle*> (get_ui_by_name("VisionRect"));

	int sight_size = neural_ai->GetSightSize();
	if (sight_size % 2 == 0)
	{
		l->SetPosition(neural_ai->GetCurrentVisionPosition() * tile_size.x - (tile_size.x * 0.5));
	}
	else
	{
		l->SetPosition(neural_ai->GetCurrentVisionPosition() * tile_size.x);
	}

	l->SetSize(k2d::vi2d(sight_size * tile_size));
	l->SetActive(true);
}

void ConquestMultiplayer::GetRandomColorFromLoadedSkins(int index)
{
	std::uniform_int_distribution<int> rand_col(0, loaded_skins.size() - 1);

	skins.at(index) = loaded_skins.at(rand_col(random_engine));

	UpdateTileColors();
}

int ConquestMultiplayer::bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x_, uint8_t y_)
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

k2d::Sprite* ConquestMultiplayer::create_tile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), tile_size.x, tile_size.y, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}

k2d::Sprite* ConquestMultiplayer::CreateDefaultSprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), scaled_ui.x, scaled_ui.y, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}

k2d::Sprite* ConquestMultiplayer::create_projectile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), 8, 8, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}


k2d::Text* ConquestMultiplayer::create_text(std::string text, float scale, float depth)
{
	return new k2d::Text(text, font1, 0, 0, scale, depth, k2d::Color(255,255, 0, 255), sprite_batch);
}

k2d::Text* ConquestMultiplayer::create_text(std::string text, k2d::vi2d position, float scale, float depth)
{
	return new k2d::Text(text, font1, position.x, position.y, scale, depth, k2d::Color(255), sprite_batch);
}

UIBase* ConquestMultiplayer::get_ui_by_name(std::string name)
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

UIButton* ConquestMultiplayer::get_button_by_name(std::string name)
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


int ConquestMultiplayer::SendCommandToServer(uint8_t code, bool& should_do_something)
{
	// ready = code 80
	int offset = 0;
	uint8_t c = code;

	// Write the header
	packet_header_t h;
	h.id = player_id;
	h.packet_type = packet_type_command;
	write(buffer, &h, sizeof(h), &offset);

	// Write the data
	write(buffer, &c, sizeof(c), &offset);

	int result = sendto(sock, buffer, offset, 0, (struct sockaddr*)&server_address, address_length);
	should_do_something = false;
	return result;
}