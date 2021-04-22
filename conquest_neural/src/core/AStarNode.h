#pragma once
#include <core/Engine.h>

class AStarNode
{
public:
	AStarNode(k2d::vi2d position, AStarNode* prev) 
	{
		this->position = position;
		this->prev = prev;
	}
	~AStarNode() 
	{
		
	}

	float getG()const {
		float res = 0.0f;
		const AStarNode* scan = this;
		while (scan->prev)
		{
			res += (abs(position.x - scan->position.x) + abs(position.y - scan->position.y)) == 1 ? 1.0f : 1.41f;
			scan = scan->prev;
		}
		return res;
	}

	float getH(const k2d::vi2d& end) const {
		float dx = (float) (end.x - position.x);
		float dy = (float) (end.y - position.y);
		return sqrtf(dx * dx + dy * dy);
	}

	float getF(const k2d::vi2d& end) const{
		return getG()*5 + getH(end);
	}

	AStarNode* prev;
	k2d::vi2d position;
	

};