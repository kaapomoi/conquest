#pragma once

#include <core/Engine.h>
#include <ui/UIClickable.h>

class UIButton : public UIClickable
{
public:
	UIButton(std::string name, k2d::vi2d position, k2d::vi2d size, k2d::Sprite* sprite, k2d::Text* text);
	virtual ~UIButton();

	virtual void Update(double dt);
	virtual void OnClick() override;
	virtual void OnClick(k2d::vf2d rel_click_pos) override;


	// Setters
	virtual void SetPosition(k2d::vi2d new_pos);
	virtual void SetTextOffset(k2d::vi2d offset);

	virtual void SetSprite(k2d::Sprite* sprite);
	virtual void SetText(k2d::Text* text);
	virtual void SetName(std::string name);

	virtual void SetActualText(std::string new_text);
	virtual void SetIsActive(bool a);

	// Getters
	k2d::vf2d GetPosition() { return position; }
	std::string GetName() { return name; }
	k2d::Sprite* GetSprite() { return sprite; }
	k2d::Text* GetText() { return text; }
	k2d::vi2d GetSize() { return size; }
	bool IsActive() { return active; }

	std::string GetActualText() { return text->GetText(); }

protected:
	std::string		name;
	k2d::Sprite*	sprite;
	k2d::Text*		text;

	k2d::vi2d		position;
	k2d::vi2d		size;
	k2d::vi2d		text_pos_offset;
	bool			active;

};