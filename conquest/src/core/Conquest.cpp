#include <core/Conquest.h>

const int packet_type_input_data = 1;
const int packet_type_tile_data = 2;
const int packet_type_command = 3;
const int packet_type_invalid_color = 4;
const int packet_type_turn_change = 5;
const int packet_type_game_end = 6;

Conquest::Conquest()
{
	window_title = "Conquest";
	window_width = 1200;
	window_height = 900;
	v_sync = true;

	face = 0;
	ft = 0;

	fps_target = 60.0f;
	dt = 0.0000000001;

	camera_mvmt_speed = 200.f;

	if (init_engine() == 0) 
	{
		// Init game
		init_game();

		//
		//create_ui();
		//create_ui_unit_card();
		

		// run game
		run();
	}
}

Conquest::~Conquest()
{
}

int Conquest::init_engine()
{
	engine = new k2d::Engine(window_title, window_width, window_height, v_sync);
	sprite_batch = engine->GetSpriteBatch();
	engine->AddShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag", "2d",
		{ "vertex_position", "vertex_color", "vertex_uv" });

	return 0;
}

int Conquest::init_game()
{
	load_texture_into_cache("empty", "Textures/tiles/square100x100.png");
	load_texture_into_cache("selected", "Textures/tiles/selection100x100.png");
	load_texture_into_cache("ss", "Textures/tiles/ss100x100.png");
	load_texture_into_cache("dot", "Textures/tiles/dot100x100.png");
	load_texture_into_cache("full", "Textures/tiles/full100x100.png");

	//// Units
	//load_texture_into_cache("vilgefortz", "Textures/units/vilgefortz100x100.png");
	//load_texture_into_cache("regis1", "Textures/units/regis_1_100x100.png");
	//load_texture_into_cache("regis2", "Textures/units/regis_2_100x100.png");

	//// Statbar
	//load_texture_into_cache("statbar", "Textures/ui/statbarcontainer100x100.png");
	//load_texture_into_cache("healthbar", "Textures/ui/healthbar100x100.png");
	//load_texture_into_cache("manabar", "Textures/ui/manabar100x100.png");

	//// Projectiles
	//load_texture_into_cache("default_proj", "Textures/projectiles/default8x8.png");

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
		loaded_skins.push_back(k2d::Color(r,g,b, 255));
	}
	myfile.close();
	 
	std::ifstream netfile("config/net.txt");
	if (!netfile) {
		k2d::KUSI_ERROR("SKIN LOADING ERROR Q");
	}
	int a;
	int c;
	int d; 
	int port;
	netfile >> a >> b >> c >> d >> port;
	SERVER_ADDRESS = MAKEADDRESS(a,b,c,d); //localhost
	SERVER_PORT = port;

	netfile >> a >> b >> c >> d >> port;
	address = MAKEADDRESS(a, b, c, d);
	netfile.close();
	


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


	tilemap = nullptr;

	// Send packets every 33ms = 30hz
	send_packets_every_ms = 0.033f;
	timer_counter = 0.0f;
	map_size = { 0,0 };

	// Net code
	random_engine.seed((unsigned int) time(NULL));
	std::uniform_int_distribution<int> num(1000, 1000000);
	player_id = num(random_engine);

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

	winner_index = 0;

	return 0;
}

int Conquest::create_objects()
{

	
	
	return 0;
}

int Conquest::create_ui()
{
	// origin top left of text
	/*UIElement* ui = new UIElement(k2d::vi2d(0,0), new k2d::Sprite(glm::vec2(0.0f, 0.0f), 200, 100, 20.0f,
		glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(255, 255, 255, 255), load_texture_from_cache("ui"), sprite_batch),
		create_text("Mousecoords", 0.15f, 25.0f));
	
	ui->SetTextOffset(k2d::vf2d(-75, 0));*/
	k2d::vi2d scaled_ui = tile_size * 4;

	UIElement* p1 = new UIElement("P1Score", k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
		glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(0), load_texture_from_cache("full"), sprite_batch),
		create_text("P1", 0.15f, 25.0f));
	p1->SetIsActive(false);
	ui_elements.push_back(p1);

	UIElement* p2 = new UIElement("P2Score",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3) , tile_size.y * map_size.y - scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
		glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(1), load_texture_from_cache("full"), sprite_batch),
		create_text("P2", 0.15f, 25.0f));
	p2->SetIsActive(false);
	ui_elements.push_back(p2);

	UIElement* p3 = new UIElement("P3Score",
		k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, tile_size.y * map_size.y - scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(3), load_texture_from_cache("full"), sprite_batch),
		create_text("P4", 0.15f, 25.0f));
	p3->SetIsActive(false);
	ui_elements.push_back(p3);
	
	UIElement* p4 = new UIElement("P4Score",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), scaled_ui.y / 2 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(2), load_texture_from_cache("full"), sprite_batch),
		create_text("P4", 0.15f, 25.0f));
	p4->SetIsActive(false);
	ui_elements.push_back(p4);
	
	UIElement* p5 = new UIElement("P5Score",
		k2d::vi2d(0 - scaled_ui.x + tile_size.x / 2, tile_size.y * (map_size.y / 2) - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(4), load_texture_from_cache("full"), sprite_batch),
		create_text("P5", 0.15f, 25.0f));
	p5->SetIsActive(false);
	ui_elements.push_back(p5);

	UIElement* p6 = new UIElement("P6Score",
		k2d::vi2d(tile_size.x * map_size.x + scaled_ui.x - ((tile_size.x / 2) * 3), tile_size.y * (map_size.y / 2) - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), scaled_ui.x, scaled_ui.y, 20.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(5), load_texture_from_cache("full"), sprite_batch),
		create_text("P6", 0.15f, 25.0f));
	p6->SetIsActive(false);
	ui_elements.push_back(p6);


	return 0;
}

int Conquest::run()
{
	engine->RunExternalLoop(fps_target);
	main_loop();
	
	return 0;
}

void Conquest::UpdateTileColors()
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

void Conquest::UpdateButtonColors()
{
	// Buttons
	for (size_t i = 0; i < num_colors; i++)
	{
		buttons.at(i)->GetSprite()->SetColor(skins.at(i));
	}
}

void Conquest::UpdateScoreboardColors()
{
	// Score "boards"
	// 8 = max players
	for (size_t i = 0; i < 8; i++)
	{
		if (players[i].id != 0)
		{
			std::string scorep1 = std::to_string(players[i].tiles_owned);
			std::string ui_name = "P" + std::to_string(i+1) + "Score";
			get_ui_by_name(ui_name)->GetSprite()->SetColor(skins.at(players[i].num_owned));
			get_ui_by_name(ui_name)->SetIsActive(true);
		}
	}
}

void Conquest::UpdateBarColors()
{
	float num_player_tiles = 0;
	float num_tiles = map_size.x * map_size.y;
	float num_remaining_tiles = num_tiles;
	for (int i = 0; i < 8; i++)
	{
		if (players[i].id == player_id)
		{
			this_player_index = i;
		}
		else if(players[i].id != 0)
		{
			num_remaining_tiles -= players[i].tiles_owned;
		}
	}
	
	float start = -tile_size.x / 2;
	float map_width = tile_size.x * map_size.x;
	bar.clear();

	// player of this client always goes on the left
	float width = ceil((players[this_player_index].tiles_owned * map_width) / num_tiles);
	UIElement* bar_ui0 = new UIElement("Bar",
		k2d::vi2d(ceil(start + width / 2), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), width, tile_size.y * 2, 30.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(players[this_player_index].num_owned), load_texture_from_cache("full"), sprite_batch),
		0);
	start += width;
	bar.push_back(bar_ui0);
	num_remaining_tiles -= players[this_player_index].tiles_owned;

	width = ceil((num_remaining_tiles * map_width) / num_tiles);
	UIElement* darkout = new UIElement("Bar",
		k2d::vi2d(ceil(start + width / 2), tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
		new k2d::Sprite(glm::vec2(0.0f, 0.0f), width, tile_size.y * 2, 30.0f,
			glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(40, 40, 40, 255), load_texture_from_cache("full"), sprite_batch),
		0);
	start += width;
	bar.push_back(darkout);

	for (int i = 0; i < 8; i++)
	{
		if (players[i].id != player_id && players[i].id != 0)
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

}

void Conquest::GetRandomColorFromLoadedSkins(int index)
{
	std::uniform_int_distribution<int> rand_col(0, loaded_skins.size() - 1);

	skins.at(index) = loaded_skins.at(rand_col(random_engine));

	UpdateTileColors();
	UpdateButtonColors();
	UpdateScoreboardColors();
	UpdateBarColors();
}

int Conquest::bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x_, uint8_t y_)
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
			the_queue.push({ x + 1, y});
			v[y][x + 1] = 1;
		}
		// Left
		if (valid_tile(x - 1, y, map_size)
			&& v[y][x - 1] == 0
			&& ((tilemap[y][x - 1].color == our_color && tilemap[y][x - 1].owner == owner) || tilemap[y][x - 1].color == new_color))
		{
			the_queue.push({ x - 1, y});
			v[y][x - 1] = 1;
		}
	}

	return num_visited;
}

void Conquest::HandleAI()
{
	num_of_each_color.clear();

	// If its the AI's turn, do ai stuff
	if (turn_id == player_id)
	{
		// BFS on the tilemap for each color
		for (size_t i = 0; i < num_colors; i++)
		{
			num_of_each_color.push_back(std::make_pair(bfs(players[this_player_index].num_owned, i, this_player_index, sc[this_player_index].x, sc[this_player_index].y), i));
		}
		// highest first
		std::sort(num_of_each_color.begin(), num_of_each_color.end(),
			[](const std::pair<int, int>& a, const std::pair<int, int>& b) -> bool
			{
				return a.first > b.first;
			}
		);

		auto it = num_of_each_color.begin();
		while (it != num_of_each_color.end())
		{
			if (taken_colors.at(it->second))
			{
				it = num_of_each_color.erase(it);
			}
			else
			{
				it++;
			}
		
		}

	}

}

int Conquest::main_loop()
{
	while (engine->GetRunning())
	{
		engine->PreRender("2d");

		

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

				delete[] tilemap;

				// Create tilemap
				tilemap = new tile * [map_size.y];
				for (uint8_t i = 0; i < map_size.y; ++i)
					tilemap[i] = new tile[map_size.x];
				// Populate the tilemap
				for (uint8_t y = 0; y < map_size.y; y++)
				{
					for (uint8_t x = 0; x < map_size.x; x++)
					{
						/*uint8_t new_tile;
						read(buffer, &new_tile, sizeof(uint8_t), &offset);

						tilemap[y][x].color = (new_tile & 0xf0) >> 4;
						tilemap[y][x].owner = (new_tile & 0x0f);*/
						tile new_tile;
						read(buffer, &new_tile, sizeof(tile), &offset);
						tilemap[y][x].color = new_tile.color;
						tilemap[y][x].owner = new_tile.owner;
					}
				}

				// Update Tile and Button Colors
				UpdateTileColors();


				//Clear UI elements, except mousecoords
				for (int i = 0; i < buttons.size(); i++)
				{
					delete buttons.at(i);
				}
				buttons.resize(0);


				// Create UI Color Buttons for input
				for (size_t i = 0; i < num_colors; i++)
				{
					UIElement* button = new UIElement("Button",
						k2d::vi2d(-tile_size.x / 2 + tile_size.x * 2 + tile_size.x * 4 * i, -tile_size.y * 4 + tile_size.y / 2),
						new k2d::Sprite(glm::vec2(0.0f, 0.0f),
							tile_size.x * 4, tile_size.y * 4, 30.0f,
							glm::vec4(0.f, 0.f, 1.f, 1.f), skins.at(i),
							load_texture_from_cache("full"), sprite_batch),
						0);
					// If the color is taken, hide the button
					if (taken_colors.at(i) == true)
					{
						button->SetIsActive(false);
					}
					buttons.push_back(button);
				}

				if (scoreboard_init == false)
				{
					create_ui();
					scoreboard_init = true;
				}

				// 8 = max players
				for (size_t i = 0; i < 8; i++)
				{
					if (players[i].id != 0)
					{
						std::string scorep1 = std::to_string(players[i].tiles_owned);
						std::string ui_name = "P" + std::to_string(i + 1) + "Score";
						get_ui_by_name(ui_name)->SetActualText(scorep1);
						get_ui_by_name(ui_name)->SetTextOffset(k2d::vf2d(((float)scorep1.length() / 2.0f) * -20.f / 2, -5));
						get_ui_by_name(ui_name)->GetSprite()->SetColor(skins.at(players[i].num_owned));
						get_ui_by_name(ui_name)->SetIsActive(true);
					}
				}
				UpdateBarColors();

				int board_width = tile_size.x * map_size.x - 2 * tile_size.x;
				int board_height = -2 * tile_size.y + tile_size.y * map_size.y;

				engine->SetCameraPosition((k2d::vf2d(board_width / 2, board_height / 2)));
				
				sc.resize(8);
				sc[0] = { 0,0 };
				sc[1] = { map_size.x - 1, map_size.y - 1 };
				sc[2] = { 0, map_size.y - 1 };
				sc[3] = { map_size.x - 1 ,0 };
				sc[4] = { 0, map_size.y / 2 - 1 };
				sc[5] = { map_size.x - 1, map_size.y / 2 - 1 };
				sc[6] = { map_size.x / 2 - 1, 0 };
				sc[7] = { map_size.x / 2 - 1, map_size.y - 1 };

				if (turn_id == player_id && AI_CONTROL)
				{
					HandleAI();
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

				// Loop through players and search the one with winner id
				for (size_t i = 0; i < 8; i++)
				{
					if (players[i].id == winner_id)
					{
						winner_index = i;
						
						break;
					}
				}

				turns_text = new UIElement("NRTurns",
					k2d::vi2d((tile_size.x * map_size.x) / 2, tile_size.y * (map_size.y) + tile_size.y * 3 - tile_size.y / 2),
					new k2d::Sprite(glm::vec2(0.0f, 0.0f), 0.0f, 0.0f, 20.0f,
						glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0, 0, 0, 0), load_texture_from_cache("empty"), sprite_batch),
					create_text("NRTurns: ", 0.15f, 50.0f));

				std::string turn_end_text = "Turns played: " + std::to_string(turns_played);
				turns_text->SetActualText(turn_end_text);
				turns_text->SetTextOffset(k2d::vf2d(((float)turn_end_text.length() / 2.0f) * -20.f / 2, -2));
				turns_text->SetIsActive(true);

				game_over = true;
				break;
			}
			default:
				break;
			}

		}

		for (GameObject* tile : tiles)
		{
			// Draw tile
			tile->Update(dt);
		}
		// End Updating gameobjects
		blink_timer += dt;
		if (blink_timer >= blink_every_second && game_over)
		{
			if (ui_elements.at(winner_index)->IsActive())
			{
				ui_elements.at(winner_index)->SetIsActive(false);
			}
			else
			{
				ui_elements.at(winner_index)->SetIsActive(true);
			}
			blink_timer = 0;
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

		if (turns_text != nullptr)
		{
			turns_text->Update(dt);
		}

		for (UIElement* b : buttons)
		{
			b->DestroyChildren();
			if (player_id != turn_id)
			{
				// Its not this clients turn, gray out the colors?
				UIElement* grayout = new UIElement("Grayout", b->GetPosition(),
					new k2d::Sprite(glm::vec2(0.0f, 0.0f),
						tile_size.x * 4, tile_size.y * 4, 50.0f,
						glm::vec4(0.f, 0.f, 1.f, 1.f), k2d::Color(0,0,0,168),
						load_texture_from_cache("full"), sprite_batch),
					0);
				b->AddChild(grayout);
			}

			b->Update(dt);
		}
		
		if (AI_CONTROL)
		{
			if (num_of_each_color.size() > try_to_play_best)
			{
				input_num = num_of_each_color.at(try_to_play_best).second;
			}
			else
			{
				input_num = -1;
			}
			if (game_over)
			{
				AI_CONTROL = false;
			}
			if (turn_id != player_id)
			{
				input_num = -1;
			}
		}

		update_input();
		dt = engine->Update();
	}

	
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

	return 0;
}



void Conquest::update_input()
{
	if (engine->GetInputManager().IsKeyPressed(SDLK_a))
	{
		AI_CONTROL = true;
		std::cout << "AI CONTROL ENABLED\n";
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_d))
	{
		should_send_drop_all = true;
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_r))
	{
		should_send_ready = true;
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_o))
	{
		should_send_reset = true;
	}

	input_pressed = false;
	if (engine->GetInputManager().IsButtonPressed(SDL_BUTTON_LEFT))
	{
		// Check which button is pressed.
		k2d::vi2d click_pos = engine->ScreenToWorld(engine->GetMouseCoords());
		//std::cout << "click: " << click_pos << "\n";
		for (size_t i = 0; i < num_colors; i++)
		{
			if (buttons.at(i)->IsActive())
			{
				// BOt left position
				k2d::vi2d button_pos;
				button_pos.x = buttons.at(i)->GetSprite()->GetPosition().x - buttons.at(i)->GetSprite()->GetDimensions().x / 2;
				button_pos.y = buttons.at(i)->GetSprite()->GetPosition().y - buttons.at(i)->GetSprite()->GetDimensions().y / 2;
				k2d::vi2d button_dims;
				button_dims.x = buttons.at(i)->GetSprite()->GetDimensions().x;
				button_dims.y = buttons.at(i)->GetSprite()->GetDimensions().y;
				// Check if its a hit
				if (click_pos.x > button_pos.x && click_pos.x < (button_pos.x + button_dims.x)
					&& click_pos.y > button_pos.y && click_pos.y < (button_pos.y + button_dims.y))
				{
					//std::cout << "hit: " << i << "\n";
					// If hit, break from loop
					input_pressed = true;
					input_num = i;
					break;
				}
			}
		}
	}

	if (engine->GetInputManager().IsKeyPressed(SDLK_1))
	{
		GetRandomColorFromLoadedSkins(0);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_2))
	{
		GetRandomColorFromLoadedSkins(1);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_3))
	{
		GetRandomColorFromLoadedSkins(2);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_4))
	{
		GetRandomColorFromLoadedSkins(3);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_5))
	{
		GetRandomColorFromLoadedSkins(4);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_6))
	{
		GetRandomColorFromLoadedSkins(5);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_7))
	{
		GetRandomColorFromLoadedSkins(6);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_8))
	{
		GetRandomColorFromLoadedSkins(7);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_9))
	{
		GetRandomColorFromLoadedSkins(8);
	}
	if (engine->GetInputManager().IsKeyPressed(SDLK_0))
	{
		GetRandomColorFromLoadedSkins(9);
	}
	// Random ALL skins
	if (engine->GetInputManager().IsKeyPressed(SDLK_k))
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

	if (input_pressed == false && AI_CONTROL == false)
	{
		input_num = -1;
	}
}



int Conquest::SendCommandToServer(uint8_t code, bool& should_do_something)
{
	if (turns_text != nullptr)
	{
		turns_text->SetIsActive(false);
	}
	
	for (size_t i = 0; i < ui_elements.size(); i++)
	{
		if (players[i].id != 0)
		{
			ui_elements.at(i)->SetIsActive(true);
		}
	}

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

	int result = sendto(sock, buffer, offset, 0, (struct sockaddr*) & server_address, address_length);
	should_do_something = false;
	return result;
}

int Conquest::load_texture_into_cache(const char* friendly_name, std::string filename)
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

k2d::GLTexture Conquest::load_texture_from_cache(const char* friendly_name)
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

k2d::Sprite* Conquest::create_tile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), tile_size.x, tile_size.y, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}

k2d::Sprite* Conquest::create_projectile_sprite(const char* texture_name, k2d::Color color, float depth)
{
	return new k2d::Sprite(glm::vec2(0.0f), 8, 8, depth,
		glm::vec4(0.f, 0.f, 1.f, 1.f), color, load_texture_from_cache(texture_name), sprite_batch);
}


k2d::Text* Conquest::create_text(std::string text, float scale, float depth)
{
	return new k2d::Text(text, font1, 0, 0, scale, depth, k2d::Color(255), sprite_batch);
}

UIElement* Conquest::get_ui_by_name(std::string name)
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
std::map<GLchar, k2d::Character> Conquest::LoadFont(const char* _file)
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
std::map<GLchar, k2d::Character> Conquest::LoadChars()
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
