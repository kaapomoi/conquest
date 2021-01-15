#pragma once

#include <core/UIElement.h>
#include <core/Unit.h>

class UIUnitCard : public UIElement
{
public:
	UIUnitCard(std::string name, k2d::vi2d position, float width, float height, k2d::Sprite* sprite, Unit* unit, k2d::SpriteBatch* sb, std::map<GLchar, k2d::Character>& font);
	virtual ~UIUnitCard();

	virtual void Update(double dt);

	void SetPosition(k2d::vf2d new_pos);
	void SetUnitSprite(k2d::Sprite* sprite);
	void SetUnit(Unit* u);
	void SetUnitSpritePosition(k2d::vf2d new_pos);

private:
	k2d::Text*		name_text;
	k2d::Text*		health_text;
	k2d::Text*		mana_text;
	k2d::Text*		ad_text;
	k2d::Text*		as_text;
	k2d::Text*		state_text;
	k2d::Sprite*	unit_sprite;

	std::map<GLchar, k2d::Character> font;

	k2d::vi2d		position;
	float			width;
	float			height;
	k2d::SpriteBatch* sb;

	Unit*			unit;
};