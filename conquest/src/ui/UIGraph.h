#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>

class UIGraph : public UIBase
{
public:
	UIGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points, float max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb);
	virtual ~UIGraph();

	virtual void Update(double dt);

	virtual void UpdateBarPositions();

	virtual void SetPosition(k2d::vf2d new_pos);
	virtual void SetIsButton(bool is_but);
	virtual void SetIsHit(bool is_hit);

	virtual void AddDataPoint(float data);

	virtual void SetDataToFollow(std::vector<float>* data);
	virtual void SetMaxDataValue(float max_data_value);

	virtual void SetMaxDataPoints(int max_data_points);

	virtual void AddHorizontalLine(float percent_of_max_value, k2d::Color color);

	virtual void AddSprite(k2d::Sprite* sprite);
	virtual void AddText(k2d::Text* text);

	virtual void SetBackground(k2d::Color bg_color);

	virtual void SetIsActive(bool a);

	k2d::vf2d GetPosition() { return position; }
	k2d::vi2d GetSize() { return size; }
	std::vector<k2d::Sprite*> GetSprites() { return bar_sprites; }
	std::vector<k2d::Text*> GetTexts() { return texts; }
	bool IsActive() { return active; }

protected:
	k2d::SpriteBatch*			sb;
	k2d::GLTexture				bar_texture;
	std::string					name;
	k2d::Sprite*				background;
	std::vector<k2d::Sprite*>	horizontal_line_sprites;
	std::vector<k2d::Sprite*>	bar_sprites;
	std::vector<k2d::Text*>		texts;
	std::vector<float>*			data_points;

	int max_data_points;
	float max_data_value;

	k2d::vf2d		position;
	k2d::vi2d		size;
	bool			should_be_gray;
	bool			active;
	bool			is_button;
	bool			is_hit;
};