#pragma once

#include <core/Engine.h>
#include <ui/UIClickable.h>
#include <ui/UIBase.h>

class UIClickableLabel : public UIClickable, public UIBase
{
public:
	UIClickableLabel(std::string name, std::string label_text, k2d::vi2d position, k2d::vi2d text_offset, k2d::vi2d size, k2d::GLTexture tex, k2d::SpriteBatch* sb,
		std::map<GLchar, k2d::Character>& _font,
		float _scale, float _depth, k2d::Color _color);
	~UIClickableLabel();

	void Update(double dt)override;

	void SetPosition(k2d::vf2d new_pos) override;
	void SetIsHit(bool is_hit);

	// Just dont look at this, ok?
	void SetVariable(int* i);
	void SetVariable(float* f);
	void SetVariable(double* d);

	void SetBaseMultiplier(int i);
	void SetBaseMultiplier(float f);
	void SetBaseMultiplier(double d);

	void SetPrintPrecision(int decimal_points);

	void SetModifiable(bool m);

	void SetVariableMultiplier(int mul);

	void SetPrettyPrintFunc(pretty_print_func func);

	void SetBackground(k2d::Color bg_color);

	void SetIsActive(bool a);

	void OnClick(k2d::vf2d relative_hit_pos) override;

	k2d::Sprite* GetSprite() { return background; }
	bool IsActive() { return active; }
	bool IsHit() { return is_hit; }

private:
	k2d::SpriteBatch*			sb;
	k2d::GLTexture				bar_texture;
	k2d::Sprite*				background;
	k2d::Label*					label;

	bool			modifiable;
	int				variable_multiplier;
	bool			active;
	bool			is_hit;
};