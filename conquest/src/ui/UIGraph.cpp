#include <ui/UIGraph.h>

UIGraph::UIGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points, int max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb)
{
	this->name = name;
	this->position = position;
	this->size = size;
	this->max_data_points = max_data_points;
	this->max_data_value = max_data_value;
	this->bar_texture = bar_tex;
	this->sb = sb;
	this->should_be_gray = false;
	this->active = true;
	this->is_button = false;
	this->is_hit = false;
	this->data_points = new std::vector<float>();
}

UIGraph::~UIGraph()
{
	for (k2d::Sprite* s : bar_sprites)
	{
		delete s;
	}

	for (k2d::Text* t : texts)
	{
		delete t;
	}

	delete background;
	delete data_points;
}

// Recursively updates this and children
void UIGraph::Update(double dt)
{
	if (active)
	{
		for (k2d::Sprite* s : bar_sprites)
		{
			s->Tick();
		}
		for (k2d::Sprite* line : horizontal_line_sprites)
		{
			line->Tick();
		}
		for (k2d::Text* t : texts)
		{
			t->Update();
		}
		if (background)
		{
			background->Tick();
		}
	}
}

void UIGraph::UpdateBarPositions()
{
	float offset = 0.0f;
	float width = (float)size.x / (float)max_data_points;

	// start position = left
	float start_x = position.x - size.x * 0.5f + width / 2;
	for (size_t i = 0; i < bar_sprites.size(); i++)
	{
		float height = (data_points->at(i) / max_data_value) * size.y;
		bar_sprites.at(i)->SetWidth(width);
		bar_sprites.at(i)->SetPosition(glm::vec2(start_x + offset, position.y + height * 0.5f));
		bar_sprites.at(i)->SetHeight(height);
		offset += width;
		if (should_be_gray && data_points->size() >= max_data_points)
		{
			if (i % 2 == 0)
			{
				bar_sprites.at(i)->SetColor(k2d::Color(200, 255));
			}
			else
			{
				bar_sprites.at(i)->SetColor(k2d::Color(255, 255));
			}
		}
		else
		{
			if (i % 2 == 0)
			{
				bar_sprites.at(i)->SetColor(k2d::Color(255, 255));
			}
			else
			{
				bar_sprites.at(i)->SetColor(k2d::Color(200, 255));
			}
		}
	}

}

void UIGraph::SetPosition(k2d::vf2d new_pos)
{
	position = new_pos;
}

void UIGraph::SetIsButton(bool is_but)
{
	is_button = is_but;
}

void UIGraph::SetIsHit(bool is_hit)
{
	this->is_hit = is_hit;
}

void UIGraph::AddDataPoint(float data)
{
	if (data_points->size() >= max_data_points)
	{
		// Remove the first element
		data_points->erase(data_points->begin());

		// Delete the sprite for the first element
		delete bar_sprites.front();
		bar_sprites.erase(bar_sprites.begin());
	}

	if (data >= max_data_value)
	{
		max_data_value = data;
	}
	data_points->push_back(data);

	k2d::Sprite* new_bar = new k2d::Sprite(glm::vec2(position.x, position.y), size.x / data_points->size(), size.y, 25.0f, glm::vec4(0,0,1,1), k2d::Color(255, 255), bar_texture, sb);
	bar_sprites.push_back(new_bar);
	should_be_gray = !should_be_gray;
	UpdateBarPositions();
}

void UIGraph::SetDataToFollow(std::vector<float>* data)
{
	for (k2d::Sprite* s : bar_sprites)
	{
		delete s;
	}
	bar_sprites.clear();

	this->data_points = data;
	this->max_data_points = data->size();
	for (size_t i = 0; i < max_data_points; i++)
	{
		k2d::Sprite* new_bar = new k2d::Sprite(glm::vec2(position.x, position.y), 0.0f, 0.0f, 25.0f, glm::vec4(0, 0, 1, 1), k2d::Color(255, 255), bar_texture, sb);
		bar_sprites.push_back(new_bar);
	}
	UpdateBarPositions();
}

void UIGraph::SetMaxDataValue(int max_data_value)
{
	this->max_data_value = max_data_value;
}

void UIGraph::AddHorizontalLine(float percent_of_max_value, k2d::Color color)
{
	float bot_level = position.y;
	float width = size.x;
	float height = 1.0f;

	float level = bot_level + (size.y * percent_of_max_value);

	k2d::Sprite* line = new k2d::Sprite(glm::vec2(position.x, level), width, height, 26.0f, glm::vec4(0,0,1,1), color, bar_texture, sb);

	horizontal_line_sprites.push_back(line);
}

void UIGraph::AddSprite(k2d::Sprite* sprite)
{
	bar_sprites.push_back(sprite);
}

void UIGraph::AddText(k2d::Text* text)
{
	texts.push_back(text);
}

void UIGraph::SetBackground(k2d::Color bg_color)
{
	if (!background)
	{
		background = new k2d::Sprite(glm::vec2(position.x, position.y + size.y * 0.5f), size.x, size.y, 21.0f, glm::vec4(0, 0, 1, 1), bg_color, bar_texture, sb);
	}
	else
	{
		background->SetColor(bg_color);
	}
}

void UIGraph::SetName(std::string name)
{
	this->name = name;
}

void UIGraph::SetIsActive(bool a)
{
	this->active = a;
}