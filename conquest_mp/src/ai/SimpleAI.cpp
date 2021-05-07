#include <ai/SimpleAI.h>

SimpleAI::SimpleAI(std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr):
	AI(tilemap_ptr, taken_colors_ptr)
{
	rand_engine.seed(time(NULL));
}

SimpleAI::~SimpleAI()
{
}

int SimpleAI::Update()
{
	// Determine the best move
	int send_this = -1;

	send_this = HandleTurn();

	return send_this;
}

void SimpleAI::SetStartingPosition(k2d::vi2d position)
{
	starting_position = position;
}

void SimpleAI::SetMapSize(k2d::vi2d map_size)
{
	this->map_size = map_size;
}

void SimpleAI::SetCurrentColorOwned(int color)
{
	current_color_owned = color;
}

int SimpleAI::bfs(uint8_t our_color, uint8_t new_color, uint8_t owner, uint8_t x_, uint8_t y_)
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
			&& (((*tilemap)[y + 1][x].color == our_color && (*tilemap)[y + 1][x].owner == owner) || (*tilemap)[y + 1][x].color == new_color))
		{
			the_queue.push({ x, y + 1 });
			v[y + 1][x] = 1;
		}
		// Up
		if (valid_tile(x, y - 1, map_size)
			&& v[y - 1][x] == 0
			&& (((*tilemap)[y - 1][x].color == our_color && (*tilemap)[y - 1][x].owner == owner) || (*tilemap)[y - 1][x].color == new_color))
		{
			the_queue.push({ x, y - 1 });
			v[y - 1][x] = 1;
		}
		// Right
		if (valid_tile(x + 1, y, map_size)
			&& v[y][x + 1] == 0
			&& (((*tilemap)[y][x + 1].color == our_color && (*tilemap)[y][x + 1].owner == owner) || (*tilemap)[y][x + 1].color == new_color))
		{
			the_queue.push({ x + 1, y });
			v[y][x + 1] = 1;
		}
		// Left
		if (valid_tile(x - 1, y, map_size)
			&& v[y][x - 1] == 0
			&& (((*tilemap)[y][x - 1].color == our_color && (*tilemap)[y][x - 1].owner == owner) || (*tilemap)[y][x - 1].color == new_color))
		{
			the_queue.push({ x - 1, y });
			v[y][x - 1] = 1;
		}
	}

	return num_visited;
}

bool SimpleAI::valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size)
{
	if (x < 0 || y < 0) {
		return false;
	}
	if (x >= map_size.x || y >= map_size.y) {
		return false;
	}
	return true;
}

int SimpleAI::HandleTurn()
{
	bfs_amount_of_each_color.clear();

	// BFS on the tilemap for each color
	for (size_t i = 0; i < taken_colors->size(); i++)
	{
		if (!taken_colors->at(i))
		{
			bfs_amount_of_each_color.push_back(std::make_pair(bfs(current_color_owned, i, which_player_am_i, starting_position.x, starting_position.y), i));
		}
	}
	// highest first
	std::sort(bfs_amount_of_each_color.begin(), bfs_amount_of_each_color.end(),
		[](const std::pair<int, int>& a, const std::pair<int, int>& b) -> bool
		{
			return a.first > b.first;
		}
	);

	SetCurrentColorOwned(bfs_amount_of_each_color.at(0).second);

	return bfs_amount_of_each_color.at(0).second;
}
