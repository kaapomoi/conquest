#include <ui/UIBase.h>

UIBase::UIBase(std::string name):
	name(name)
{

}

UIBase::~UIBase()
{

}

void UIBase::SetName(std::string n)
{
	this->name = n;
}
