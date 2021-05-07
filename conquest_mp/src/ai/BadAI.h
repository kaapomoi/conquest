#pragma once
#include <core/Engine.h>
#include <ai/AI.h>

class BadAI : public AI
{
public:
	using Random = effolkronium::random_static;
	BadAI(std::vector<std::vector<tile>>* tilemap_ptr, std::vector<bool>* taken_colors_ptr);
	virtual ~BadAI();

	int Update() override;


protected:

};