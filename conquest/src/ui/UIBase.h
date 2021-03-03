#pragma once

#include <core/Engine.h>

class UIBase
{
public:
	UIBase(std::string name, k2d::vf2d position, k2d::vf2d size, float depth);
	virtual ~UIBase();

	virtual void Update(double dt) = 0;
	


	virtual void SetName(std::string n) { this->name = n; }
	virtual void SetDepth(float d) { this->depth = d; }
	virtual void SetPosition(k2d::vf2d pos) { this->position = pos; }
	virtual void SetSize(k2d::vf2d size) { this->size = size; }
	virtual void SetActive(bool a) { this->active = a; }

	std::string GetName() { return name; }
	float GetDepth() { return depth; }

	k2d::vf2d GetPosition() { return position; }
	k2d::vf2d GetSize() { return size; }

	bool IsActive() { return active; }

protected:
	std::string name;

	k2d::vf2d	position;
	k2d::vf2d	size;
	float		depth;

	bool		active;
};