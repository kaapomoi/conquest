#include <ui/UIToggleButton.h>

UIToggleButton::UIToggleButton(std::string name, k2d::vi2d position, k2d::vi2d size, k2d::Sprite* sprite, k2d::Text* text, k2d::Sprite* darkout_sprite, bool toggled): 
	UIButton(name, position, size, sprite, text)
{
	this->darkout_sprite = darkout_sprite;
	this->toggled = toggled;

	if (darkout_sprite != nullptr)
	{
		darkout_sprite->SetPosition(glm::vec3(position.x, position.y, 0.0f));
		darkout_sprite->SetWidth(size.x);
		darkout_sprite->SetHeight(size.y);
	}
}

UIToggleButton::~UIToggleButton()
{

}

void UIToggleButton::Update(double dt)
{
	if (toggled)
	{
		darkout_sprite->Tick();	
	}
	UIButton::Update(dt);
}

void UIToggleButton::OnClick(k2d::vf2d relative_click_pos)
{
	// Toggle bool
	toggled = !toggled;

	UIButton::OnClick(relative_click_pos);
}
