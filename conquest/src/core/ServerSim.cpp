#include <core/ServerSim.h>

ServerSim::ServerSim(k2d::vi2d map_size, int num_colors) :
    num_turns(DEFAULT_NUM_TURNS),
    whose_turn(DEFAULT_WHOSE_TURN),
    event_id_running(1),
    match_id_running(0),
    map_size(map_size),
    NR_OF_COLORS(num_colors),
    game_in_progress(false)
{
    rand_engine.seed(time(NULL));

    // Initialize the starting positions based on the map size
    s_pos.resize(MAX_PLAYERS);
    s_pos[0] = { 0, 0 };
    s_pos[1] = { map_size.x - 1, map_size.y - 1 };
    s_pos[2] = { 0, map_size.y - 1 };
    s_pos[3] = { map_size.x - 1, 0 };

}

ServerSim::~ServerSim()
{

}

bool ServerSim::ConnectToServer(int player_id)
{
    // Check if we already have this player in the server.
    bool found = false;
    int found_index = 0;
    for (unsigned int i = 0; i < players.size(); i++) {
        if (player_id == players[i].id) {
            found = true;
            found_index = i;
            k2d::KUSI_DEBUG("Player with id %d already connected, returning...\n", player_id);
            return true;
        }
    }

    // If its a new player, add it to the players vector
    if (found == false)
    {
        // Add the new player to the vector
        players.push_back(new_player(player_id));
        player_ids.push_back(player_id);
        found = true;
        found_index = players.size() - 1;
    }

	return true;
}

void ServerSim::DisconnectFromServer(int player_id)
{
    player_ids.erase(std::remove(player_ids.begin(), player_ids.end(), player_id), player_ids.end());
    players.erase(std::remove_if(players.begin(), players.end(), [player_id](player_t p) {return p.id == player_id; }), players.end());
}

void ServerSim::ReceiveInput(int player_id, int recv_num)
{
    int id = player_id;
    // Get id and find players from players vec first

    bool found = false;
    int found_index = 0;
    for (unsigned int i = 0; i < players.size(); i++) {
        if (id == players[i].id) {
            found = true;
            found_index = i;
            break;
        }
    }

    // Don't do anything with strangers inputs
    if (!found)
    {
        k2d::KUSI_DEBUG("Player with id %i not found...returning\n", player_id);
        return;
    }

    // If we find a player with the sender id, continue handling the turn

    // Check whether the received number is valid
    if (recv_num >= 0 && recv_num < NR_OF_COLORS) {
        if (id == players.at(whose_turn).id)
        {
            // Handle the change if color is valid
            if (!taken_colors.at(recv_num))
            {
                // Change ownership of color
                taken_colors.at(players.at(whose_turn).num_owned) = false;
                taken_colors.at(recv_num) = true;
                players.at(whose_turn).num_owned = recv_num;

                uint8_t x = s_pos[whose_turn].x;
                uint8_t y = s_pos[whose_turn].y;

                //k2d::KUSI_DEBUG("Client with id: %d sent number: %d\n", id, recv_num);

                // Add the color to the turn history
                turn_history.push_back(recv_num);
                

                // Change the color of already owned tiles
                flood_fill_color_change(map_size, x, y, whose_turn, recv_num);
                // Change the ownership of all same colored tiles
                players.at(whose_turn).tiles_owned = bfs_owner_change(map_size, x, y, whose_turn, recv_num);

                // Change the turn
                whose_turn++;
                if (whose_turn >= players.size())
                {
                    num_turns++;
                    whose_turn = 0;
                }
                // Check whether no unowned tiles are left
                int num_tiles_owned = 0;
                int most_tiles_index = 0;
                for (size_t i = 0; i < players.size(); i++)
                {
                    num_tiles_owned += players.at(i).tiles_owned;
                    if (players.at(i).tiles_owned >= players.at(most_tiles_index).tiles_owned)
                    {
                        most_tiles_index = i;
                    }
                }

                // check if all the tiles are owned 
                if (num_tiles_owned >= (map_size.x * map_size.y))
                {
                    Event e(EventType::GAME_OVER, event_id_running++, player_ids);
                    
                    // Sub the spectator too
                    e.SubscribeAClientId(SPECTATOR_ID);
                    // Format:
                    // <winner_id>:<num_turns>

                    e.SetData(std::to_string(players.at(most_tiles_index).id));
                    e.AppendToData(std::to_string(num_turns));
                    e.AppendToData(std::to_string(match_id_running));

                    // push the end game event to the event queue
                    event_queue.AddItem(e);
                    game_in_progress = false;
                }
                else
                {
                    Event e(EventType::TURN_CHANGE, event_id_running++, player_ids);
                    // Sub the spectator too
                    e.SubscribeAClientId(SPECTATOR_ID);
                    // Format:
                    // <whose_turn.id>

                    e.SetData(std::to_string(players.at(whose_turn).id));

                    // push the end game event to the event queue
                    event_queue.AddItem(e);
                }


            }
            else
            {
                int offset = 0;

                printf("Not a valid color to change to\n");

                // Only subscribe the sender of the invalid color to the event
                Event e(EventType::INVALID_COLOR, event_id_running++, std::vector<int> {players[found_index].id});

                // Format:
                // <recv_num>

                e.SetData(std::to_string(recv_num));

           
                // Send the invalid color packet to the player who sent it
                printf("Invalid packet sent to player %d, id: %d, num: %d\n", found_index, players[found_index].id, recv_num);
                event_queue.AddItem(e);
            }
        }
        else
        {
            printf("Not correct players turn client_id: %d\n", id);
        }
    }


}

int ServerSim::StartGame()
{
    game_in_progress = true;
    match_id_running++;

    turn_history.clear();

    create_game(players.size());

    init_players_and_taken_colors();

    return 1;
}

std::vector<std::vector<tile>> ServerSim::GetBoardState()
{
    // Returns a copy of tilemap
    return tilemap;
}

std::vector<bool> ServerSim::GetTakenColors()
{
    return taken_colors;
}

Event ServerSim::GetNextEventFromQueue(int client_id)
{
    return event_queue.GetNextEvent(client_id);
}

std::vector<player_t> ServerSim::GetPlayers()
{
    return players;
}

std::vector<k2d::vi2d> ServerSim::GetStartingPositions()
{
    return s_pos;
}

std::vector<int> ServerSim::GetTurnHistory()
{
    return turn_history;
}

int ServerSim::GetTurnsPlayed()
{
    return num_turns;
}

bool ServerSim::GetGameInProgress()
{
    return game_in_progress;
}

// Creates a new player with the given id. Inits other variables with 0!!
player_t ServerSim::new_player(int id)
{
    player_t player;
    player.id = id;
    player.num_owned = 0;
    player.tiles_owned = 0;
    return player;
}

void ServerSim::create_game(int num_players)
{
    // Random color generator
    std::uniform_int_distribution<int> num_gen(0, NR_OF_COLORS - 1);

    // POSSIBLE: Read mapsize and nr_colors from conf file here:
    // ...

    
   /* s_pos[4] = { 0, map_size.y / 2 - 1 };
    s_pos[5] = { map_size.x - 1, map_size.y / 2 - 1 };
    s_pos[6] = { map_size.x / 2 - 1, 0 };
    s_pos[7] = { map_size.x / 2 - 1, map_size.y - 1 };*/
    
    // Populate the tilemap
    tilemap.resize(map_size.y);
    for (uint8_t y = 0; y < map_size.y; y++)
    {
        tilemap.at(y).resize(map_size.x);
        for (uint8_t x = 0; x < map_size.x; x++)
        {
            // Create a tile, give it 9 owner value = no owner
            // Cast to unsigned char to avoid confusion
            tilemap[y][x] = { (uint8_t)num_gen(rand_engine), 9 };
        }
    }

    // Change ownership of correct tiles for players
    // Force change surrounding tiles to some other color than the players color.
    for (uint8_t i = 0; i < num_players; i++)
    {
        // Set origin point for players
        tilemap[s_pos[i].y][s_pos[i].x] = { i, i };

        // Get adjacent tiles and own them
        std::vector<k2d::vi2d> adjacent_should_own = get_adjacent_tiles(map_size, s_pos[i]);
        for (size_t j = 0; j < adjacent_should_own.size(); j++)
        {
            // Set the ownership and color of tiles that should be changed
            tilemap[adjacent_should_own.at(j).y][adjacent_should_own.at(j).x] = { i,i };

            // Get all adjacent tiles that should not be the same as starting tile
            std::vector<k2d::vi2d> foreign_tiles = get_adjacent_foreign_tiles(map_size, i, adjacent_should_own.at(j));
            for (size_t k = 0; k < foreign_tiles.size(); k++)
            {
                // Force the tiles that should not be owned / the same color to not be the same color
                force_change_to(foreign_tiles.at(k).x, foreign_tiles.at(k).y, i, num_gen);
            }
        }

    }

    // Set all the colors free
    taken_colors.resize(NR_OF_COLORS);
    for (size_t i = 0; i < taken_colors.size(); i++)
    {
        taken_colors.at(i) = false;
    }
    whose_turn = 0;
    num_turns = 0;

    // TODO: Remember to set taken colors for players...

}

void ServerSim::init_players_and_taken_colors()
{
    for (size_t i = 0; i < players.size(); i++)
    {
        // Setup taken colors
        taken_colors.at(i) = true;
        // Players own the colors in order
        players.at(i).num_owned = i;
        // see create_game    
        players.at(i).tiles_owned = 3;       
    }
}

std::vector<k2d::vi2d> ServerSim::get_adjacent_tiles(k2d::vi2d map_size, k2d::vi2d pos)
{
    
    // Initialize candidates with strictly adjacent tiles
    std::vector<k2d::vi2d> candidates;
    candidates.push_back(pos + k2d::vi2d(0, 1));
    candidates.push_back(pos + k2d::vi2d(1, 0));
    candidates.push_back(pos + k2d::vi2d(0, -1));
    candidates.push_back(pos + k2d::vi2d(-1, 0));
    std::vector<k2d::vi2d> res;
    for (k2d::vi2d c : candidates)
    {
        // If its a valid tile, add it to the result vector
        if (valid_tile(c.x, c.y, map_size))
        {
            res.push_back(c);
        }
    }

    return res;
}

void ServerSim::force_change_to(uint8_t x, uint8_t y, uint8_t forbid, std::uniform_int_distribution<int>& num_gen)
{
    // Create random colors until its not the forbidden color
    while (tilemap[y][x].color == forbid)
    {
        printf("Creating a new color at %i,%i\n", x, y);
        tilemap[y][x].color = num_gen(rand_engine);
    }
}

std::vector<k2d::vi2d> ServerSim::get_adjacent_foreign_tiles(k2d::vi2d map_size, uint8_t owner, k2d::vi2d pos)
{
    // Initialize candidates with strictly adjacent tiles
    std::vector<k2d::vi2d> candidates;
    candidates.push_back(pos + k2d::vi2d(0, 1));
    candidates.push_back(pos + k2d::vi2d(1, 0));
    candidates.push_back(pos + k2d::vi2d(0, -1));
    candidates.push_back(pos + k2d::vi2d(-1, 0));
    std::vector<k2d::vi2d> res;
    for (k2d::vi2d c : candidates)
    {
        // If its a valid tile, add it to the result vector
        if (valid_tile(c.x, c.y, map_size) && tilemap[c.y][c.x].owner != owner)
        {
            res.push_back(c);
        }
    }

    return res;
    
}

void ServerSim::flood_fill_color_change(k2d::vi2d map_size, uint8_t x, uint8_t y, uint8_t owner, uint8_t new_color)
{
    // Base cases
    if (x < 0 || x >= map_size.x || y < 0 || y >= map_size.y)
        return;
    if (tilemap[y][x].owner != owner)
        return;
    if (tilemap[y][x].color == new_color)
        return;

    // Replace the color at (x, y)
    tilemap[y][x].color = new_color;

    // Recur for north, east, south and west
    flood_fill_color_change(map_size, x + 1, y, owner, new_color);
    flood_fill_color_change(map_size, x - 1, y, owner, new_color);
    flood_fill_color_change(map_size, x, y + 1, owner, new_color);
    flood_fill_color_change(map_size, x, y - 1, owner, new_color);
}

int ServerSim::bfs_owner_change(k2d::vi2d map_size, uint8_t x, uint8_t y, uint8_t new_owner, uint8_t color)
{
    int num_visited = 0;
    // Visited array
    uint8_t v[100][100];
    memset(v, 0, sizeof(v));

    std::queue<std::pair<uint8_t, uint8_t>> the_queue;

    the_queue.push({ x, y });
    v[y][x] = 1;


    // run until the queue is empty
    while (!the_queue.empty())
    {
        // Extrating front pair
        std::pair<uint8_t, uint8_t> coord = the_queue.front();
        int x = coord.first;
        int y = coord.second;

        tilemap[y][x].owner = new_owner;
        num_visited++;

        // Poping front pair of queue
        the_queue.pop();

        // Down
        if (valid_tile(x, y + 1, map_size)
            && v[y + 1][x] == 0
            && tilemap[y + 1][x].color == color)
        {
            the_queue.push({ x, y + 1 });
            v[y + 1][x] = 1;
        }
        // Up
        if (valid_tile(x, y - 1, map_size)
            && v[y - 1][x] == 0
            && tilemap[y - 1][x].color == color)
        {
            the_queue.push({ x, y - 1 });
            v[y - 1][x] = 1;
        }
        // Right
        if (valid_tile(x + 1, y, map_size)
            && v[y][x + 1] == 0
            && tilemap[y][x + 1].color == color)
        {
            the_queue.push({ x + 1, y });
            v[y][x + 1] = 1;
        }
        // Left
        if (valid_tile(x - 1, y, map_size)
            && v[y][x - 1] == 0
            && tilemap[y][x - 1].color == color)
        {
            the_queue.push({ x - 1, y });
            v[y][x - 1] = 1;
        }
    }

    return num_visited;
 
}

void ServerSim::Update()
{
    // Update the queue
    event_queue.Update();
}

k2d::vi2d ServerSim::GetMapSize()
{
    return map_size;
}

bool ServerSim::valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size)
{
    if (x < 0 || y < 0) {
        return false;
    }
    if (x >= map_size.x || y >= map_size.y) {
        return false;
    }
    return true;
}

