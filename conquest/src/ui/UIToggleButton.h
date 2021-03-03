#pragma once

#include <core/Engine.h>
#include <ui/UIButton.h>


class UIToggleButton : public UIButton
{
public:
	UIToggleButton(std::string name, k2d::vi2d position, k2d::vi2d size, float depth, k2d::Sprite* sprite, k2d::Text* text, k2d::Sprite* darkout_sprite, bool toggled = false);
	~UIToggleButton();

	void Update(double dt) override;

	void ToggleFuncSideways();
	void ToggleFuncOnOff();

	void OnClick(k2d::vf2d relative_click_pos);

	void ResetToUntoggledState();

	void SetDarkoutActive(bool ac);

	k2d::Sprite* GetDarkoutSprite() { return darkout_sprite; }
 
private:
	// This sprite is rendered when toggled is true
	k2d::Sprite* darkout_sprite;

	// Initial state is false
	bool		toggled;
};