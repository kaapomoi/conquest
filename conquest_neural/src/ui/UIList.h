#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>

class UIList : public UIBase
{
public:
	UIList(std::string name, k2d::vf2d position,  k2d::vi2d text_offset, k2d::vf2d size, float depth, float row_height, int max_rows, k2d::GLTexture tex, k2d::SpriteBatch* sb,
		std::map<GLchar, k2d::Character>& _font,
		float _scale, k2d::Color _color);
	~UIList();

	void Update(double dt) override;

	void UpdatePositions();
	void AddBackground(k2d::Color color);

	void SetVectorToFollow(std::vector<int>* v);

	void UpdateListValues();

private:
	float					row_height;
	int						rows;
	int						max_rows;

	bool					needs_update;

	float text_scale;
	k2d::vi2d				text_offset;

	std::vector<int>*		vector_to_follow;

	k2d::Sprite*			background_sprite;
	std::vector<k2d::Text*>	text_lines;

	std::map<GLchar, k2d::Character>& font;

	k2d::GLTexture			texture;
	k2d::SpriteBatch*		sb;
};