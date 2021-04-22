#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>

class UIScoreBar : public UIBase
{
public:
	UIScoreBar(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb);
	~UIScoreBar();

	void Update(double dt);

	void UpdateBarPositions();

	void SetMaxTileCount(int max_tiles);
	
	void AddBackground(k2d::Color color);

	void SetVariablePointers(int* p0_t, k2d::Color* p0_c, int* p1_t, k2d::Color* p1_c);

	void AddMarker(int* value_ptr, k2d::Color color);

	void UpdateBar();

	void SetPosition(k2d::vf2d pos) override;

private:
	k2d::Sprite*				p0_score_sprite;
	k2d::Sprite*				p1_score_sprite;
	std::vector<int*>			marker_positions;
	std::vector<k2d::Sprite*>	marker_sprites;

	k2d::Sprite*				background_sprite;

	k2d::GLTexture				bar_texture;
	k2d::SpriteBatch*			sb;

	int							tiles_max;

	int*						p0_tiles;
	k2d::Color*					p0_color;

	int*						p1_tiles;
	k2d::Color*					p1_color;

	bool						should_update_bars;
};