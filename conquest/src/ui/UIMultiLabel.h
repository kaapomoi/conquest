#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>

class UIMultiLabel : public UIBase
{
public:
	UIMultiLabel(std::string name, k2d::vi2d position, k2d::vi2d size, float row_height, float scale, float depth, std::map<GLchar, k2d::Character>& _font, k2d::GLTexture tex, k2d::SpriteBatch* _sprite_batch);
	virtual ~UIMultiLabel();

	void AddLabel(std::string name, std::string label_text, int* variable);

	void AddBackground(k2d::Color color = k2d::Color(255));

	void Update(double dt);

private:
	k2d::vi2d					position;
	k2d::vi2d					size;
	k2d::Sprite*				background;
	k2d::SpriteBatch*			sb;
	k2d::GLTexture				bg_tex;
	std::map<GLchar, k2d::Character>& font;

	std::vector<k2d::Label*>	label_rows;

	float						scale;
	float						depth;

	float						row_height;
};