#include <ai/NeuralAI.h>

NeuralAI::NeuralAI(const char* file_name, std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr) :
	AI(tilemap_ptr, taken_colors_ptr)
{
	int layer_count = 0;
	std::ifstream file(file_name);
	file >> sight_size >> x_weight >> y_weight >> layer_count;

	for (size_t i = 0; i < layer_count; i++)
	{
		int num_neurons = 0;
		file >> num_neurons;
		net_topology.push_back(num_neurons);
	}

	neural_net = NeuralNet(net_topology);
	std::vector<std::vector<Neuron>>& net = neural_net.GetLayers();

	for (int i = 0; i < layer_count; i++)
	{
		for (int j = 0; j < net_topology.at(i); j++)
		{
			//int num_outputs = i >= layer_count - 1 ? 0: net_topology.at(i+1);
			int num_weights = (int)net[i][j].output_weights.size();

			file >> net[i][j].bias_weight >> net[i][j].my_index;

			if (num_weights == 0)
			{
				net[i][j].is_output = true;
			}
			else
			{
				net[i][j].is_output = false;
			}

			for (size_t k = 0; k < num_weights; k++)
			{
				file >> net[i][j].output_weights[k];
			}
		}
	}

	rand_engine.seed(time(NULL));
	tiles_owned = 0;
	try_best = 0;
	game_won = false;
	file.close();
}

NeuralAI::~NeuralAI()
{

}

int NeuralAI::Update()
{
	int res = -1;
	// Send the input values in to the neural net
	std::vector<double> board_state_single_dimension;
	std::vector<std::vector<tile>> board_state = *tilemap;


	/// calculate the furthest tile from starting pos and place the middle of the sight rect on that tile
	get_end_tiles();
	k2d::vi2d best_tile;

	float best_score = 0.0f;
	k2d::vi2d start_pos = sc[which_player_am_i];
	std::sort(end_tiles.begin(), end_tiles.end(), [start_pos, this](const k2d::vi2d a, const k2d::vi2d b) -> bool
		{
			float dx_a = a.x - start_pos.x;
			float dy_a = a.y - start_pos.y;
			float score_a = abs(dx_a) * x_weight + abs(dy_a) * y_weight;
			float dx_b = b.x - start_pos.x;
			float dy_b = b.y - start_pos.y;
			float score_b = abs(dx_b) * x_weight + abs(dy_b) * y_weight;
			return score_a > score_b;
		});


	std::vector<float> weights;
	float sum = 0.0f;
	for (size_t i = 0; i < end_tiles.size(); i++)
	{
		float val = 100 / (float)(i + 10);
		weights.push_back(val);
		sum += val;
	}

	float sample_at = Random::get(0.0f, sum);
	float iterative = 0.0f;
	int index_of_selected_tile = 0;
	for (size_t i = 0; i < end_tiles.size(); i++)
	{
		iterative += weights.at(i);
		if (sample_at < iterative || i == end_tiles.size() - 1)
		{
			index_of_selected_tile = i;
			break;
		}
	}

	best_tile = end_tiles.at(index_of_selected_tile);
	//best_tile = end_tiles.at(Random::get(0, (int)weights.size()-1));

	vision_grid_position = best_tile;
	k2d::vi2d& t = vision_grid_position;
	int s_h_l = sight_size / 2;
	int s_h_r = sight_size - s_h_l;

	int margin_left = t.x - s_h_l;
	int margin_right = board_state[0].size() - (t.x + s_h_r);
	int margin_bot = t.y - s_h_l;
	int margin_top = board_state.size() - (t.y + s_h_r);

	if (margin_left < 0)
	{
		t.x -= margin_left;
	}
	if (margin_right < 0)
	{
		t.x += margin_right;
	}
	if (margin_bot < 0)
	{
		t.y -= margin_bot;
	}
	if (margin_top < 0)
	{
		t.y += margin_top;
	}

	//// Convert the 2d array into a single dimension array of doubles
	for (size_t i = t.y - s_h_l; i < t.y + s_h_r; i++)
	{
		for (size_t j = t.x - s_h_l; j < t.x + s_h_r; j++)
		{
			double in_value = board_state[i][j].color;
			double min_in_range = 0.0;
			double max_in_range = 1.0 * (taken_colors->size() - 1);
			double min_out_range = -1.0;
			double max_out_range = 1.0;

			double result = k2d::map_to_range(in_value, min_in_range, max_in_range, min_out_range, max_out_range);


			board_state_single_dimension.push_back(result);

					
		}
	}

	// Input the board state into the net
	//neural_net.FeedForward(board_state_single_dimension, taken_colors, false);
	neural_net.FeedForward(board_state_single_dimension, *taken_colors, false);


	// Get the results from the net
	neural_net.GetResults(result_vec);
	results_map.clear();

	for (size_t i = 0; i < result_vec.size(); i++)
	{
		//std::cout << "res " << i << ": " << result_vec[i] << "\n";
		if (taken_colors->at(i) == false)
		{
			results_map.push_back(std::make_pair(i, result_vec[i]));
		}
	}

	// highest first
	std::sort(results_map.begin(), results_map.end(),
		[](const std::pair<int, double>& a, const std::pair<int, double>& b) -> bool
		{
			return a.second > b.second;
		}
	);


	res = results_map[try_best].first;

	// Send the number to the server
	return res;
	
}

NeuralNet* NeuralAI::GetNeuralNet()
{
	return &neural_net;
}

void NeuralAI::get_end_tiles()
{
	auto& tiles = *tilemap;
	end_tiles.clear();

	k2d::vi2d map_size = {(int)tiles[0].size(), (int)tiles.size()};

	int num_visited = 0;
	// Visited array
	uint8_t v[100][100];
	memset(v, 0, sizeof(v));

	std::queue<std::pair<uint8_t, uint8_t>> the_queue;

	sc.resize(8);
	sc[0] = { 0,0 };
	sc[1] = { map_size.x - 1, map_size.y - 1 };
	sc[2] = { 0, map_size.y - 1 };
	sc[3] = { map_size.x - 1 ,0 };
	sc[4] = { 0, map_size.y / 2 - 1 };
	sc[5] = { map_size.x - 1, map_size.y / 2 - 1 };
	sc[6] = { map_size.x / 2 - 1, 0 };
	sc[7] = { map_size.x / 2 - 1, map_size.y - 1 };

	k2d::vi2d starting_pos = sc[which_player_am_i];

	the_queue.push({ starting_pos.x, starting_pos.y });
	v[starting_pos.y][starting_pos.x] = 1;

	int x = -1;
	int y = -1;
	// run until the queue is empty
	while (!the_queue.empty())
	{
		bool already_added = false;
		// Extrating front pair
		std::pair<uint8_t, uint8_t> coord = the_queue.front();
		x = coord.first;
		y = coord.second;

		num_visited++;

		// Poping front pair of queue
		the_queue.pop();

		//// Down
		//if (valid_tile(x, y + 1, map_size)
		//	&& v[y + 1][x] == 0
		//	&& (tiles[y + 1][x].owner == which_player_am_i))
		//{
		//	the_queue.push({ x, y + 1 });
		//	v[y + 1][x] = 1;
		//	new_found = true;
		//}
		// Down
		if (valid_tile(x, y + 1, map_size) && v[y + 1][x] == 0)
		{
			if (tiles[y + 1][x].owner == which_player_am_i)
			{
				the_queue.push({ x, y + 1 });
				v[y + 1][x] = 1;
			}
			else if(already_added == false)
			{
				already_added = true;
				end_tiles.push_back({x, y});
			}
		}
		//// Up
		//if (valid_tile(x, y - 1, map_size)
		//	&& v[y - 1][x] == 0
		//	&& (tiles[y - 1][x].owner == which_player_am_i))
		//{
		//	the_queue.push({ x, y - 1 });
		//	v[y - 1][x] = 1;
		//	new_found = true;
		//}
		// Up
		if (valid_tile(x, y - 1, map_size) && v[y - 1][x] == 0)
		{
			if (tiles[y - 1][x].owner == which_player_am_i)
			{
				the_queue.push({ x, y - 1 });
				v[y - 1][x] = 1;
			}
			else if(already_added == false)
			{
				already_added = true;
				end_tiles.push_back({ x, y });
			}
		}
		//// Right
		//if (valid_tile(x + 1, y, map_size)
		//	&& v[y][x + 1] == 0
		//	&& (tiles[y][x + 1].owner == which_player_am_i))
		//{
		//	the_queue.push({ x + 1, y });
		//	v[y][x + 1] = 1;
		//}
		// RIGHT
		if (valid_tile(x + 1, y, map_size) && v[y][x + 1] == 0)
		{
			if (tiles[y][x + 1].owner == which_player_am_i)
			{
				the_queue.push({ x + 1, y });
				v[y][x + 1] = 1;
			}
			else if (already_added == false)
			{
				already_added = true;
				end_tiles.push_back({ x, y });
			}
		}
		//// Left
		//if (valid_tile(x - 1, y, map_size)
		//	&& v[y][x - 1] == 0
		//	&& (tiles[y][x - 1].owner == which_player_am_i))
		//{
		//	the_queue.push({ x - 1, y });
		//	v[y][x - 1] = 1;
		//}
		// RIGHT
		if (valid_tile(x - 1, y, map_size) && v[y][x - 1] == 0)
		{
			if (tiles[y][x - 1].owner == which_player_am_i)
			{
				the_queue.push({ x - 1, y });
				v[y][x - 1] = 1;
			}
			else if (already_added == false)
			{
				already_added = true;
				end_tiles.push_back({ x, y });
			}
		}
	}

}

bool NeuralAI::valid_tile(uint8_t x, uint8_t y, k2d::vi2d map_size)
{
	if (x < 0 || y < 0) {
		return false;
	}
	if (x >= map_size.x || y >= map_size.y) {
		return false;
	}
	return true;
}