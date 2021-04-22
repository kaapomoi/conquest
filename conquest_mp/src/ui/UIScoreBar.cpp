#include <ui/UIScoreBar.h>

UIScoreBar::UIScoreBar(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb):
	UIBase(name, position, size, depth), background_sprite(nullptr), bar_texture(bar_tex), sb(sb),
	p0_color(nullptr), p0_tiles(nullptr), p1_color(nullptr), p1_tiles(nullptr)
{
	p0_score_sprite = new k2d::Sprite(position, size, depth, k2d::Color(255), bar_texture, sb);
	p1_score_sprite = new k2d::Sprite(position, size, depth, k2d::Color(255), bar_texture, sb);
	tiles_max = 0;
	should_update_bars = true;
}

UIScoreBar::~UIScoreBar()
{
	delete p0_score_sprite;
	delete p1_score_sprite;

	for (k2d::Sprite* s : marker_sprites)
	{
		delete s;
	}
	delete background_sprite;
}

void UIScoreBar::Update(double dt)
{
	if (active)
	{
		if (should_update_bars)
		{
			UpdateBarPositions();
		}

		p0_score_sprite->Tick();
		p1_score_sprite->Tick();

		if (background_sprite)
		{
			background_sprite->Tick();
		}

		for (k2d::Sprite* s : marker_sprites)
		{
			s->Tick();
		}
	}
}

void UIScoreBar::UpdateBarPositions()
{
	int mid_tile_count = tiles_max - *p0_tiles - *p1_tiles;
	float start_pos_left = position.x - size.x * 0.5f;
	float start = start_pos_left;

	// p0
	float width = ceil((*p0_tiles * size.x) / tiles_max);
	p0_score_sprite->SetPosition(glm::vec2(start + width * 0.5f, position.y));
	p0_score_sprite->SetWidth(width);
	p0_score_sprite->SetColor(*p0_color);
	start += width;

	// middle part (empty)
	width = ceil((mid_tile_count * size.x) / tiles_max);
	start += width;

	// p1
	width = ceil((*p1_tiles * size.x) / tiles_max);
	p1_score_sprite->SetPosition(glm::vec2(start + width * 0.5f, position.y));
	p1_score_sprite->SetWidth(width);
	p1_score_sprite->SetColor(*p1_color);
	
	for (size_t i = 0; i < marker_sprites.size(); i++)
	{
		width = 1.0f;
		start = start_pos_left + (*marker_positions.at(i) * size.x / tiles_max);

		marker_sprites.at(i)->SetPosition(glm::vec2(start, position.y));
		marker_sprites.at(i)->SetWidth(width);
	}


}

void UIScoreBar::SetMaxTileCount(int max_tiles)
{
	tiles_max = max_tiles;
}

void UIScoreBar::AddBackground(k2d::Color color)
{
	if (!background_sprite)
	{
		background_sprite = new k2d::Sprite(position, size, depth - 0.5f, color, bar_texture, sb);
	}
	else
	{
		background_sprite->SetColor(color);
	}
}

void UIScoreBar::SetVariablePointers(int* p0_t, k2d::Color* p0_c, int* p1_t, k2d::Color* p1_c)
{
	p0_tiles = p0_t;
	p0_color = p0_c;
	p1_tiles = p1_t;
	p1_color = p1_c;
}

void UIScoreBar::AddMarker(int* value_ptr, k2d::Color color)
{
	marker_positions.push_back(value_ptr);
	marker_sprites.push_back(new k2d::Sprite(position, size, depth + 0.5f, color, bar_texture, sb));
}

void UIScoreBar::UpdateBar()
{
	should_update_bars = true;
}

void UIScoreBar::SetPosition(k2d::vf2d pos)
{
	if (background_sprite)
	{
		background_sprite->SetPosition(pos);
	}

	UIBase::SetPosition(pos);
}
