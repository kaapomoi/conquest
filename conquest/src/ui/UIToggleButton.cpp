#include <ui/UIToggleButton.h>

UIToggleButton::UIToggleButton(std::string name, k2d::vi2d position, k2d::vi2d size, k2d::Sprite* sprite, k2d::Text* text, k2d::Sprite* darkout_sprite, bool toggled): 
	UIButton(name, position, size, sprite, text)
{
	this->darkout_sprite = darkout_sprite;
	this->toggled = toggled;

	if (darkout_sprite != nullptr)
	{
		darkout_sprite->SetPosition(glm::vec3(position.x, position.y, 0.0f));
	}
}

UIToggleButton::~UIToggleButton()
{

}

void UIToggleButton::Update(double dt)
{
	darkout_sprite->Tick();	
	
	UIButton::Update(dt);
}

void UIToggleButton::ToggleFuncSideways()
{
	float x_offset = size.x / 4;
	if (toggled)
	{
		darkout_sprite->SetPosition(glm::vec2(position.x - x_offset, position.y));
	}
	else
	{
		darkout_sprite->SetPosition(glm::vec2(position.x + x_offset, position.y));
	}
}

void UIToggleButton::ToggleFuncOnOff()
{
	if (toggled)
	{
		darkout_sprite->SetActive(true);
	}
	else
	{
		darkout_sprite->SetActive(false);
	}
}

void UIToggleButton::OnClick(k2d::vf2d relative_click_pos)
{
	// Toggle bool
	toggled = !toggled;

	UIButton::OnClick(relative_click_pos);
}

void UIToggleButton::ResetToUntoggledState()
{
	toggled = false;
}

void UIToggleButton::SetDarkoutActive(bool ac)
{
	darkout_sprite->SetActive(ac);
}
