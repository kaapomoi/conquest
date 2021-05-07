#include <ai/BadAI.h>

BadAI::BadAI(std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr):
	AI(tilemap_ptr, taken_colors_ptr)
{
}

BadAI::~BadAI()
{
}

int BadAI::Update()
{
	// Check the server event queue for updates


	return Random::get(0, (int)taken_colors->size());
}
