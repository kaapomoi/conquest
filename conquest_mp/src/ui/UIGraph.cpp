#include <ui/UIGraph.h>

UIGraph::UIGraph(std::string name, k2d::vi2d position, k2d::vi2d size, float depth, int max_data_points, float max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb):
	UIBase(name, position, size, depth)
{
	this->max_data_points = max_data_points;
	this->max_data_value = max_data_value;
	this->bar_texture = bar_tex;
	this->sb = sb;
	this->should_be_gray = false;
	this->active = true;
	this->is_button = false;
	this->is_hit = false;
	this->background = nullptr;
	this->trend_line = nullptr;
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


void UIGraph::Update(double dt)
{
	if (active)
	{
		for (k2d::Text* t : texts)
		{
			t->Update();
		}
		for (k2d::Sprite* s : bar_sprites)
		{
			s->Tick();
		}
		for (k2d::Sprite* line : horizontal_line_sprites)
		{
			line->Tick();
		}
		if (background)
		{
			background->Tick();
		}
		if (trend_line)
		{
			trend_line->Tick();
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
		bar_sprites.at(i)->SetPosition(glm::vec2(start_x + offset, position.y - size.y * 0.5f + height * 0.5f));
		bar_sprites.at(i)->SetDepth(depth);
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

	if (trend_line)
	{
		// Calculate Trendline
		CalculateTrendLine();
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

	k2d::Sprite* new_bar = new k2d::Sprite(glm::vec2(position.x, position.y), size.x / data_points->size(), size.y, depth, glm::vec4(0,0,1,1), k2d::Color(255, 255), bar_texture, sb);
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
		k2d::Sprite* new_bar = new k2d::Sprite(glm::vec2(position.x, position.y), 0.0f, 0.0f, depth, glm::vec4(0, 0, 1, 1), k2d::Color(255, 255), bar_texture, sb);
		bar_sprites.push_back(new_bar);
	}
	UpdateBarPositions();
}

void UIGraph::SetMaxDataValue(float max_data_value)
{
	this->max_data_value = max_data_value;
}

void UIGraph::SetMaxDataPoints(int max_data_points)
{
	this->max_data_points = max_data_points;
}

void UIGraph::AddHorizontalLine(float percent_of_max_value, k2d::Color color)
{
	float bot_level = position.y - size.y * 0.5f;
	float width = size.x;
	float height = 1.0f;

	float level = bot_level + (size.y * percent_of_max_value);

	k2d::Sprite* line = new k2d::Sprite(glm::vec2(position.x, level), width, height, depth + 1.0f, glm::vec4(0,0,1,1), color, bar_texture, sb);

	horizontal_line_sprites.push_back(line);
}

void UIGraph::AddTrendLine(k2d::Color color)
{
	if (!trend_line)
	{
		trend_line = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, 1.0f, depth + 1.0f, glm::vec4(0, 0, 1, 1), color, bar_texture, sb);
	}
	else
	{
		trend_line->SetColor(color);
	}
}

void UIGraph::CalculateTrendLine()
{
	int data_points_shown = max_data_points;
	if (data_points->size() < max_data_points)
	{
		data_points_shown = data_points->size();
	}


	float a = 0.0f;
	for (size_t i = 0; i < data_points_shown; i++)
	{
		a += data_points->at(i) * i;
	}
	a *= data_points_shown;

	float b = 0.0f;
	float sum = 0.0f;
	for (size_t i = 0; i < data_points_shown; i++)
	{
		b += data_points->at(i);
		sum += i;
	}
	b *= sum;

	float c = 0.0f;
	for (size_t i = 0; i < data_points_shown; i++)
	{
		c += i*i;
	}
	c *= data_points_shown;

	float d = 0.0f;
	for (size_t i = 0; i < data_points_shown; i++)
	{
		d += i;
	}
	// squared
	d *= d;

	float slope = (a - b) / (c - d);

	float e = 0.0f;
	for (size_t i = 0; i < data_points_shown; i++)
	{
		e += data_points->at(i);
	}
	float f = 0.0f;
	for (size_t i = 0; i < data_points_shown; i++)
	{
		f += i;
	}
	f *= slope;

	// Start pos at x = 0
	float y_intercept = (e - f) / data_points_shown;

	// Find the position(y) of the line at the middle point
	float middle_pos_y = (((slope * data_points_shown * 0.5f) + y_intercept) / max_data_value) * size.y;

	trend_line->SetPosition(glm::vec2(position.x, position.y - size.y * 0.5f + middle_pos_y));
	trend_line->SetAngle(atan(slope));

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
		background = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, size.y, depth - 1.0f, glm::vec4(0, 0, 1, 1), bg_color, bar_texture, sb);
	}
	else
	{
		background->SetColor(bg_color);
	}
}