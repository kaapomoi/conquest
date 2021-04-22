#pragma once

#include <core/Engine.h>

class UIElement
{
public:
	UIElement(std::string name, k2d::vi2d position, k2d::Sprite* sprite, k2d::Text* text);
	virtual ~UIElement();

	virtual void AddChild(UIElement* child);
	virtual void DestroyChildren();

	virtual void Update(double dt);

	virtual void SetPosition(k2d::vf2d new_pos);
	virtual void SetTextOffset(k2d::vf2d offset);
	virtual void SetIsButton(bool is_but);
	virtual void SetIsHit(bool is_hit);

	virtual void SetSprite(k2d::Sprite* sprite);
	virtual void SetText(k2d::Text* text);
	virtual void SetName(std::string name);

	virtual void SetActualText(std::string new_text);
	virtual void SetIsActive(bool a);

	UIElement* GetChild() { return child; }

	k2d::vf2d GetPosition() { return position; }
	std::string GetName() { return name; }
	k2d::Sprite* GetSprite() { return sprite; }
	k2d::Text* GetText() { return text; }
	bool IsActive() { return active; }
	bool IsButton() { return is_button; }
	bool IsHit() { return is_hit; }

	std::string GetActualText() { return text->GetText(); }


protected:
	std::string		name;
	UIElement*		child;
	k2d::Sprite*	sprite;
	k2d::Text*		text;

	k2d::vf2d		position;
	k2d::vf2d		text_pos_offset;
	bool			active;
	bool			is_button;
	bool			is_hit;
};