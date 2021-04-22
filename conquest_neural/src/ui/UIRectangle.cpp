#include <ui/UIRectangle.h>

UIRectangle::UIRectangle(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::Sprite* sprite):
	UIBase(name, position, size, depth),
	sprite(sprite)
{

}

UIRectangle::~UIRectangle()
{

}

void UIRectangle::Update(double dt)
{
	if (active)
	{
		sprite->Tick();
	}
}

void UIRectangle::SetColor(k2d::Color c)
{
	sprite->SetColor(c);
}

void UIRectangle::SetDepth(float d)
{
	sprite->SetDepth(d);
	UIBase::SetDepth(d);
}

void UIRectangle::SetPosition(k2d::vf2d pos)
{
	sprite->SetPosition(pos);
	UIBase::SetPosition(pos);
}

void UIRectangle::SetSize(k2d::vf2d size)
{
	sprite->SetWidth(size.x);
	sprite->SetHeight(size.y);
	UIBase::SetSize(size);
}
