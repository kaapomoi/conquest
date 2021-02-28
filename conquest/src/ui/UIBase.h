#pragma once

#include <core/Engine.h>

class UIBase
{
public:
	UIBase(std::string name);
	virtual ~UIBase();

	void SetName(std::string n);
	std::string GetName() { return name; }

protected:
	std::string name;

};