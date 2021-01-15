#include <inttypes.h>
#include <vector>
#include <time.h>
#include <string>
#include <random>
#include <algorithm>
#include <queue>
#include <fstream>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define MAKEADDRESS(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

const int packet_type_input_data = 1;
const int packet_type_tile_data = 2;
const int packet_type_command = 3;
const int packet_type_invalid_color = 4;
const int packet_type_turn_change = 5;
const int packet_type_game_end = 6;
const int server_id = 246;

// Player struct
typedef struct {
    int id;
    uint8_t num_owned;
    int tiles_owned;
    struct sockaddr_in address;
}player_t;

// New player creator
player_t new_player(int id, int num_owned, struct sockaddr_in* address) {
    player_t player;
    player.id = id;
    player.num_owned = num_owned;
    player.tiles_owned = 3;
    memcpy(&player.address, address, sizeof(player.address));
    return player;
}

// 2d char vector
typedef struct {
    uint8_t x;
    uint8_t y;
} vc2d;

// Packet header
typedef struct {
    int packet_type;
    int id;
}packet_header_t;

// limited 2d vector struct
struct vec2d{
    int x;
    int y;
    vec2d()
    {
        this->x = 0;
        this->y = 0;
    }
    vec2d(int x, int y) {
        this->x = x;
        this->y = y;
    }
    vec2d operator+(const vec2d& other) {
        return { x + other.x, y + other.y};
    }
};

// tiles of tilemap 
typedef struct {
    uint8_t color;
    uint8_t owner;
} tile;

// Reads the data of start_position of size 'size' to the 'to' variable 
void read(char* start_position, void* to, int size, int* offset) {
    memcpy(to, start_position + *offset, size);
    *offset += size;
}

// Writes the given data of size 'size' to startposition + offset
void write(char* start_position, void* data, int size, int* offset)
{
    memcpy(start_position + *offset, data, size);
    *offset += size;
}

// Takes tilemap by ref and prints it
void print_tilemap(tile** map, vc2d size)
{
    for (uint8_t y = 0; y < size.y; y++)
    {
        for (uint8_t x = 0; x < size.x; x++)
        {
            printf("%d,", map[y][x].color);
        }
        printf("\n");
    }
    printf("\n");
}

// Takes tilemap by ref and prints it
void print_detailed_tilemap(tile** map, vc2d size)
{
    for (uint8_t y = 0; y < size.y; y++)
    {
        for (uint8_t x = 0; x < size.x; x++)
        {
            printf("[%d,%d]", map[y][x].color, map[y][x].owner);
        }
        printf("\n");
    }
    printf("\n");
}

void flood_fill_owner_change(tile** map, vc2d map_size, uint8_t x, uint8_t y, uint8_t new_owner, uint8_t color) 
{
    // Base cases
    if (x < 0 || x >= map_size.x || y < 0 || y >= map_size.y)
        return;
    if (map[y][x].color != color)
        return;
    // Return if tile is already ours or enemies'
    if (map[y][x].owner == new_owner || map[y][x].owner != 9)
        return;

    // Replace the owner at (x, y)
    map[y][x].owner = new_owner;

    // Recur for north, east, south and west
    flood_fill_owner_change(map, map_size, x + 1, y, new_owner, color);
    flood_fill_owner_change(map, map_size, x - 1, y, new_owner, color);
    flood_fill_owner_change(map, map_size, x, y + 1, new_owner, color);
    flood_fill_owner_change(map, map_size, x, y - 1, new_owner, color);
}

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

// Breadth-first-search algorithm that changes the owner of all floodfillable tiles
int bfs_owner_change(tile** map, vc2d map_size, uint8_t x, uint8_t y, uint8_t new_owner, uint8_t color)
{
    int num_visited = 0;
    // Visited array
    uint8_t v[100][100];
    memset(v, 0, sizeof(v));

    queue<pair<uint8_t, uint8_t>> the_queue;

    the_queue.push({ x, y });
    v[y][x] = 1;

    
    // run until the queue is empty
    while (!the_queue.empty())
    {
        // Extrating front pair
        pair<uint8_t, uint8_t> coord = the_queue.front();
        int x = coord.first;
        int y = coord.second;

        map[y][x].owner = new_owner;
        num_visited++;

        // Poping front pair of queue
        the_queue.pop();

        // Down
        if (valid_tile(x, y+1, map_size)
            && v[y + 1][x] == 0
            && map[y + 1][x].color == color)
        {
            the_queue.push({ x, y+1 });
            v[y + 1][x] = 1;
        }
        // Up
        if (valid_tile(x, y-1, map_size)
            && v[y - 1][x] == 0
            && map[y - 1][x].color == color)
        {
            the_queue.push({ x, y-1 });
            v[y - 1][x] = 1;
        }
        // Right
        if (valid_tile(x+1, y, map_size)
            && v[y][x + 1] == 0
            && map[y][x + 1].color == color)
        {
            the_queue.push({ x+1, y });
            v[y][x + 1] = 1;
        }
        // Left
        if (valid_tile(x-1, y, map_size)
            && v[y][x - 1] == 0
            && map[y][x - 1].color == color)
        {
            the_queue.push({ x-1, y });
            v[y][x - 1] = 1;
        }
    }

    return num_visited;
}

// Changes the color of tiles with specified owner
void flood_fill_color_change(tile** map, vc2d map_size, uint8_t x, uint8_t y, uint8_t owner, uint8_t new_color)
{
    // Base cases
    if (x < 0 || x >= map_size.x || y < 0 || y >= map_size.y)
        return;
    if (map[y][x].owner != owner)
        return;
    if (map[y][x].color == new_color)
        return;

    // Replace the color at (x, y)
    map[y][x].color = new_color;

    // Recur for north, east, south and west
    flood_fill_color_change(map, map_size, x + 1, y, owner, new_color);
    flood_fill_color_change(map, map_size, x - 1, y, owner, new_color);
    flood_fill_color_change(map, map_size, x, y + 1, owner, new_color);
    flood_fill_color_change(map, map_size, x, y - 1, owner, new_color);
}

// Takes a tilemap by reference and modifies it with params
void modify_tile(tile** map, vc2d size, vc2d pos, uint8_t c, uint8_t o)
{
    map[pos.y][pos.x] = { c,o };
}

// Forces a tile to not be a certain color
void force_change_to(tile** map, uint8_t x, uint8_t y, uint8_t forbid,
    std::mt19937& engine, std::uniform_int_distribution<int>& num_gen)
{
    // Create random colors until its not the forbidden color
    while (map[y][x].color == forbid)
    {
        printf("Trying to create new color at %i,%i\n", x, y);
        map[y][x].color = num_gen(engine);
    }
}

// Gets the adjacent tiles that are inside the map bounds
std::vector<vec2d> get_adjacent_tiles(vc2d map_size, vec2d pos) 
{
    // Initialize candidates with strictly adjacent tiles
    std::vector<vec2d> candidates;
    candidates.push_back(pos + vec2d(0, 1));
    candidates.push_back(pos + vec2d(1, 0));
    candidates.push_back(pos + vec2d(0, -1));
    candidates.push_back(pos + vec2d(-1, 0));
    std::vector<vec2d> res;
    for (vec2d c : candidates)
    {
        // If its a valid tile, add it to the result vector
        if (valid_tile(c.x, c.y, map_size))
        {
            res.push_back(c);
        }
    }

    return res;
}

// Gets the adjacent tiles that are inside the map bounds and dont have the same owner as the starting tile
std::vector<vec2d> get_adjacent_foreign_tiles(tile** map, vc2d map_size, uint8_t owner, vec2d pos)
{
    // Initialize candidates with strictly adjacent tiles
    std::vector<vec2d> candidates;
    candidates.push_back(pos + vec2d(0, 1));
    candidates.push_back(pos + vec2d(1, 0));
    candidates.push_back(pos + vec2d(0, -1));
    candidates.push_back(pos + vec2d(-1, 0));
    std::vector<vec2d> res;
    for (vec2d c : candidates)
    {
        // If its a valid tile, add it to the result vector
        if (valid_tile(c.x, c.y, map_size) && map[c.y][c.x].owner != owner)
        {
            res.push_back(c);
        }
    }

    return res;
}

/// Create a new game with given parameters
void create_game(tile** map, vc2d& map_size, uint8_t& nr_colors,
    std::vector<bool>& taken_colors, int& whose_turn, int num_players,
    std::vector<vec2d>& sc, int& num_turns)
{
    // Random number engine
    std::mt19937 RE;
    RE.seed(time(NULL));
    // Specifies the number generator for number of colors
    std::uniform_int_distribution<int> num_gen(0, nr_colors - 1);

    std::ifstream mapfile("config/map.txt");
    if (!mapfile) {
        printf("MAP CONFIG LOADING ERROR Q\n");
    }
    int a;
    int b;
    int c;
    mapfile >> a;
    nr_colors = a;

    mapfile >> b >> c;
    map_size.x = b;
    map_size.y = c;
    printf("nr_of_colors: %i - map_size: %i, %i\n", a, b, c);
    mapfile.close();

    sc[0] = { 0,0 };
    sc[1] = { map_size.x - 1, map_size.y - 1 };
    sc[2] = { 0, map_size.y - 1 };
    sc[3] = { map_size.x - 1 ,0 };
    sc[4] = { 0, map_size.y / 2 - 1 };
    sc[5] = { map_size.x - 1, map_size.y / 2 - 1 };
    sc[6] = { map_size.x / 2 - 1, 0 };
    sc[7] = { map_size.x / 2 - 1, map_size.y - 1 };

    // Populate the tilemap
    for (uint8_t y = 0; y < map_size.y; y++)
    {
        for (uint8_t x = 0; x < map_size.x; x++)
        {
            // Create a tile, give it 9 owner value = no owner
            // Cast to unsigned char to avoid confusion
            map[y][x] = { (uint8_t)num_gen(RE), 9 };
        }
    }


    for (uint8_t i = 0; i < num_players; i++)
    {
        if (i < 4)
        {
            // Set origin point for players
            map[sc[i].y][sc[i].x] = { i, i };

            // Get adjacent tiles and own them
            std::vector<vec2d> adjacent_should_own = get_adjacent_tiles(map_size, sc[i]);
            for (size_t j = 0; j < adjacent_should_own.size(); j++)
            {
                // Set the ownership and color of tiles that should be changed
                map[adjacent_should_own.at(j).y][adjacent_should_own.at(j).x] = { i,i };

                // Get all adjacent tiles that should not be the same as starting tile
                std::vector<vec2d> foreign_tiles = get_adjacent_foreign_tiles(map, map_size, i, adjacent_should_own.at(j));
                for (size_t k = 0; k < foreign_tiles.size(); k++)
                {
                    // Force the tiles that should not be owned / the same color to not be the same color
                    force_change_to(map, foreign_tiles.at(k).x, foreign_tiles.at(k).y, i, RE, num_gen);
                }
            }
        }
        else
        {
            // Set origin point for players
            map[sc[i].y][sc[i].x] = { i, i };
            // Get all adjacent tiles that should not be the same as starting tile
            std::vector<vec2d> foreign_tiles = get_adjacent_foreign_tiles(map, map_size, i, sc[i]);
            for (size_t k = 0; k < foreign_tiles.size(); k++)
            {
                // Force the tiles that should not be owned / the same color to not be the same color
                force_change_to(map, foreign_tiles.at(k).x, foreign_tiles.at(k).y, i, RE, num_gen);
            }
        }
        
    }

    taken_colors.resize(nr_colors);
    for (size_t i = 0; i < taken_colors.size(); i++)
    {
        taken_colors.at(i) = false;
    }
    whose_turn = 0;
    num_turns = 0;
}

int main(void)
{
    // Set nonblocking to not block, receive data constantly
	unsigned long NON_BLOCKING = 1;
    // Set the address to this computers IP
    uint32_t SERVER_ADDRESS; //localhost
	// Set the port to whatever this value is
    uint16_t SERVER_PORT;

    std::ifstream netfile("config/net.txt");
    if (!netfile) {
        printf("NET CONFIG LOADING ERROR Q\n");
    }
    int a;
    int b;
    int c;
    int d;
    int port;
    netfile >> a >> b >> c >> d >> port;

    SERVER_ADDRESS = MAKEADDRESS(a, b, c, d); //localhost
    SERVER_PORT = port;
    printf("SERVER ADDRESS: %i.%i.%i.%i:%i\n", a, b, c , d, SERVER_PORT);
    netfile.close();

    // Initialize map_size 
    vc2d map_size = {80, 80};

    // The starting positions for players, in order
    std::vector<vec2d> sc;
    sc.resize(8);
    
    // store whose turn it is
    int whose_turn = 0;

    // Keep track of how many turns have been played
    int num_turns = 0;

    // Set the max number of colors
    uint8_t NR_OF_COLORS = 5;
    // Taken colors vec, if true, then color at that index cant be chosen
    vector<bool> taken_colors;

    std::ifstream mapfile("config/map.txt");
    if (!mapfile) {
        printf("MAP CONFIG LOADING ERROR Q\n");
    }
    mapfile >> a;
    NR_OF_COLORS = a;

    mapfile.close();

    // The meat and bones of the game logic, Stores the colors and ownership of tiles
    tile** tilemap;

    // Create the tilemap
    tilemap = new tile * [map_size.y];
    for (uint8_t i = 0; i < map_size.y; ++i)
        tilemap[i] = new tile[map_size.x];

    // Players vector
    vector<player_t> players;
    // Creates a new game, with one player
    create_game(tilemap, map_size, NR_OF_COLORS, taken_colors, whose_turn, 1, sc, num_turns);

    // Create the socket and startup WSA
    WSAData wsa_data;
    int32_t result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Store the socket address and initialize it
    struct sockaddr_in socket_address = { 0 };
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(SERVER_ADDRESS);
    socket_address.sin_port = htons(SERVER_PORT);

    // Store the address length 
    int32_t address_length = (int32_t)sizeof(struct sockaddr_in);
    // Bind the socket to the address
    result = bind(sock, (struct sockaddr*)&socket_address, address_length);

    // Set the socket to nonblocking
    result = ioctlsocket(sock, FIONBIO, &NON_BLOCKING);

    printf("Server running\n");

    // allocate a buffer of 8k size 
    char buffer[8192] = { 0 };
    // store the sender address
    struct sockaddr_in sender_address = { 0 };
    
    // Set the header size
    const int header_size = sizeof(packet_header_t);
    while (1) {
        result = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender_address, &address_length);
        if (result > header_size)
        {
            int offset = 0;
            int id = 0;
            packet_header_t h;
            read(buffer, &h, sizeof(h), &offset);
            // Get id and find players from players vec first
            
            id = h.id;
            
            bool found = false;
            int found_index = 0;
            for (unsigned int i = 0; i < players.size(); i++) {
                if (id == players[i].id) {
                    found = true;
                    found_index = i;
                    break;
                }
            }

            // If its a normal packet, carry on doing relevant operations
            //
            //

            int recv_num = -1;

            // If its a disconnect packet, handle it
            switch (h.packet_type)
            {
                case packet_type_input_data:
                {
                    // If we're receiving input data, read the incoming input number
                    read(buffer, &recv_num, sizeof(recv_num), &offset);
                    
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

                                uint8_t x = sc[whose_turn].x;
                                uint8_t y = sc[whose_turn].y;

                                // Change the color of already owned tiles
                                flood_fill_color_change(tilemap, map_size, x, y, whose_turn, recv_num);
                                // Change the ownership of all same colored tiles
                                players.at(whose_turn).tiles_owned = bfs_owner_change(tilemap, map_size, x, y, whose_turn, recv_num);
                            
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
                                    // Set the offset to 0, IMPORTANT
                                    int offset = 0;
                                    packet_header_t h2;
                                    h2.id = server_id;
                                    h2.packet_type = packet_type_game_end;

                                    // Write the header of type turn change 
                                    write(buffer, &h2, sizeof(h2), &offset);

                                    // Write the WINNER PLAYERS id back to the sender.
                                    write(buffer, &players.at(most_tiles_index).id, sizeof(int), &offset);
                                    write(buffer, &num_turns, sizeof(num_turns), &offset);

                                    // Send the turn change packet to all players
                                    for (int i = 0; i < players.size(); i++) {
                                        sendto(sock, buffer, offset, 0, (struct sockaddr*) & players[i].address, address_length);
                                    }
                                }
                                else
                                {
                                    // Set the offset to 0, IMPORTANT
                                    int offset = 0;
                                    packet_header_t h2;
                                    h2.id = server_id;
                                    h2.packet_type = packet_type_turn_change;

                                    // Write the header of type turn change 
                                    write(buffer, &h2, sizeof(h2), &offset);

                                    // Write the turn id back to the sender.
                                    write(buffer, &players.at(whose_turn).id, sizeof(int), &offset);

                                    // Send the turn change packet to all players
                                    for (int i = 0; i < players.size(); i++) {
                                        sendto(sock, buffer, offset, 0, (struct sockaddr*) & players[i].address, address_length);
                                    }
                                }

                               
                            }
                            else
                            {
                                int offset = 0;

                                printf("Not a valid color to change to\n");
                                // Send invalid color packet to player who sent the color
                                packet_header_t h2;
                                h2.id = server_id;
                                h2.packet_type = packet_type_invalid_color;
                                
                                // Write the header with packet type invalid color
                                write(buffer, &h2, sizeof(h2), &offset);

                                // Write the invalid number back to the sender.
                                write(buffer, &recv_num, sizeof(recv_num), &offset);

                                // Send the invalid color packet to the player who sent it
                                printf("Invalid packet sent to player %d, id: %d, num: %d\n", found_index, players[found_index].id, recv_num);
                                sendto(sock, buffer, offset, 0, (struct sockaddr*) & players[found_index].address, address_length);
                               
                            }
                        }
                        else
                        {
                            printf("Not correct players turn\n");
                        }
                    }

                    break;
                }
                case packet_type_command:
                { 
                    uint8_t code = 0;
                    // Read the code from the buffer  
                    read(buffer, &code, sizeof(code), &offset);
                    printf("Code recv: %d\n", code);

                    if (code == 80)
                    {
                        // Do nothing?? :D
                    }
                    else if (code == 89)
                    { 
                        // Drop all players from the server
                        printf("Dropped all players\n");
                        players.clear();

                        continue;
                    }
                    else if (code == 90)
                    {
                        // Player disconnects, should be broadcast to all players still online
                        if (players.size() > found_index)
                        {
                            printf("Player with id %d disconnected...removing from players\n", id);
                            // Erase the disconnected player from the players vector
                            players.erase(players.begin() + found_index);


                            printf("Players vec: \n");
                            for (size_t i = 0; i < players.size(); i++)
                            {
                                printf("Player, id: %d\n", players.at(i).id);
                            }
                        }

                        continue;
                    }
                    else if (code == 100)
                    {
                        // Create a new game
                        create_game(tilemap, map_size, NR_OF_COLORS, taken_colors, whose_turn, players.size(), sc, num_turns);
                        for (size_t i = 0; i < players.size(); i++)
                        {
                            // Setup taken colors
                            taken_colors.at(i) = true;
                            // Players own the colors in order
                            players.at(i).num_owned = i;
                            // see create_game
                            if (i < 4)
                            {
                                players.at(i).tiles_owned = 3;
                            }
                            else
                            {
                                players.at(i).tiles_owned = 1;
                            }
                        }
                    }
                    break;
                }
            }

            // If the id is not found in the vector, its a new player and should be added to the players vector
            if (found == false) {
                // New player joins the server
                // Update colors taken
                taken_colors.at(players.size()) = true;
                // Add the new player to the vector
                players.push_back(new_player(id, players.size(), &sender_address));
                found = true;
                found_index = players.size() - 1;

                printf("New player entered the server: %i , player_count: %i\n", id, players.size());
            }
            if (found) {
                int num_players = players.size();
                if (num_players > 0) {

                    int offset = 0;
                    // Create the header
                    packet_header_t h3;
                    h3.id = server_id;
                    h3.packet_type = packet_type_tile_data;

                    // Write the header
                    write(buffer, &h3, sizeof(h3), &offset);

                    // Write whose turn it is                  
                    write(buffer, &players.at(whose_turn).id, sizeof(int), &offset);

                    // Total number of players 
                    write(buffer, &num_players, sizeof(num_players), &offset);
                    // Players array
                    write(buffer, &players[0], sizeof(player_t) * players.size(), &offset);
                    // Map dimensions
                    write(buffer, &map_size.x, sizeof(uint8_t), &offset);
                    write(buffer, &map_size.y, sizeof(uint8_t), &offset);

                    // Total number of colors
                    write(buffer, &NR_OF_COLORS, sizeof(uint8_t), &offset);

                    // Write taken colors to buffer
                    for (size_t i = 0; i < NR_OF_COLORS; i++)
                    {
                        if (taken_colors.at(i))
                        {
                            // Color is taken, player cant use
                            uint8_t tmp = 1;
                            write(buffer, &tmp, sizeof(uint8_t), &offset);
                        }
                        else
                        {
                            // Color is free to use
                            uint8_t tmp = 0;
                            write(buffer, &tmp, sizeof(uint8_t), &offset);
                        }
                    }

                    // Populate the tilemap
                    for (uint8_t y = 0; y < map_size.y; y++)
                    {
                        for (uint8_t x = 0; x < map_size.x; x++)
                        {
                            /*uint8_t tile_color = tilemap[y][x].color;
                            uint8_t tile_owner = tilemap[y][x].owner;

                            uint8_t packed_tile = 0x00;
                            packed_tile = (packed_tile & 0x00) | ((tile_color & 0xf) << 4);
                            packed_tile = (tile_owner & 0xf);*/


                            write(buffer, &tilemap[y][x], sizeof(tile), &offset);
                        }
                    }

                    // Send the tile data package to all the players
                    for (int i = 0; i < num_players; i++) {
                        sendto(sock, buffer, offset, 0, (struct sockaddr*) & players[i].address, address_length);
                    }
                }
            }
        }
    }

	return 0;
}