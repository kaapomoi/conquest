#include <ui/UIToggleButton.h>

UIToggleButton::UIToggleButton(std::string name, k2d::vi2d position, k2d::vi2d size, k2d::vi2d darkout_offset, float depth, k2d::Sprite* sprite, k2d::Text* text, k2d::Sprite* darkout_sprite, bool toggled):
	UIButton(name, position, size, depth, sprite, text)
{
	this->darkout_sprite = darkout_sprite;
	this->darkout_offset = darkout_offset;
	this->toggled = toggled;

	if (darkout_sprite != nullptr)
	{
		darkout_sprite->SetPosition(glm::vec3(position.x + darkout_offset.x, position.y + darkout_offset.y, 0.0f));
		darkout_sprite->SetWidth(size.x);
		darkout_sprite->SetHeight(size.y);
		darkout_sprite->SetDepth(depth + 1.0f);
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
	if (toggled)
	{
		darkout_sprite->SetPosition(glm::vec2(position.x - darkout_offset.x, position.y - darkout_offset.y));
	}
	else
	{
		darkout_sprite->SetPosition(glm::vec2(position.x + darkout_offset.x, position.y - darkout_offset.y));
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
	darkout_sprite->SetActive(false);
}

void UIToggleButton::SetDarkoutActive(bool ac)
{
	darkout_sprite->SetActive(ac);
}

void UIToggleButton::SetPosition(k2d::vf2d new_pos)
{
	this->position = new_pos;
	if (darkout_sprite)
	{
		if (toggled)
		{
			darkout_sprite->SetPosition(glm::vec2(position.x - darkout_offset.x, position.y - darkout_offset.y));
		}
		else
		{
			darkout_sprite->SetPosition(glm::vec2(position.x + darkout_offset.x, position.y - darkout_offset.y));
		}
	}
	UIButton::SetPosition(new_pos);
}