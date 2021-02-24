#pragma once

#include <core/Engine.h>
#include <ui/UIButton.h>

//typedef void (*callback_function)();

class UIToggleButton : public UIButton
{
public:
	UIToggleButton(std::string name, k2d::vi2d position, k2d::vi2d size, k2d::Sprite* sprite, k2d::Text* text, k2d::Sprite* darkout_sprite, bool toggled = false);
	~UIToggleButton();

	void Update(double dt) override;

	void OnClick(k2d::vf2d relative_click_pos);


private:
	k2d::Sprite* darkout_sprite;

	// Initial state is false
	bool		toggled;
};