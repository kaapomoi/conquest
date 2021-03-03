#include <ui/UIBase.h>

UIBase::UIBase(std::string name, k2d::vf2d position, k2d::vf2d size, float depth):
	name(name), position(position), size(size), depth(depth), active(true)
{

}

UIBase::~UIBase()
{

}