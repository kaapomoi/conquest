#pragma once

#include <core/Engine.h>
#include <ui/UIClickable.h>
#include <ui/UIBase.h>

class UIButton : public UIClickable, public UIBase
{
public:
	UIButton(std::string name, k2d::vi2d position, k2d::vi2d size, float depth, k2d::Sprite* sprite, k2d::Text* text);
	virtual ~UIButton();

	virtual void Update(double dt)override;
	virtual void OnClick() override;
	virtual void OnClick(k2d::vf2d rel_click_pos) override;


	// Setters
	virtual void SetPosition(k2d::vf2d new_pos) override;
	virtual void SetTextOffset(k2d::vi2d offset);

	virtual void SetSprite(k2d::Sprite* sprite);
	virtual void SetText(k2d::Text* text);

	virtual void SetActualText(std::string new_text);

	// Getters
	k2d::Sprite* GetSprite() { return sprite; }
	k2d::Text* GetText() { return text; }

	std::string GetActualText() { return text->GetText(); }


protected:
	k2d::Sprite*	sprite;
	k2d::Text*		text;

	k2d::vi2d		text_pos_offset;

};