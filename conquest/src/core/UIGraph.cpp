#include <core/UIGraph.h>

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
}

UIGraph::~UIGraph()
{
	for (k2d::Sprite* s : sprites)
	{
		delete s;
	}

	for (k2d::Text* t : texts)
	{
		delete t;
	}

	delete background;
}

// Recursively updates this and children
void UIGraph::Update(double dt)
{
	if (active)
	{
		for (k2d::Sprite* s : sprites)
		{
			s->Tick();
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
	for (size_t i = 0; i < data_points.size(); i++)
	{
		float height = data_points.at(i) / max_data_value * size.y;
		sprites.at(i)->SetWidth(width);
		sprites.at(i)->SetPosition(glm::vec2(start_x + offset, position.y + height * 0.5f));
		sprites.at(i)->SetHeight(height);
		offset += width;
		if (should_be_gray && data_points.size() >= max_data_points)
		{
			if (i % 2 == 0)
			{
				sprites.at(i)->SetColor(k2d::Color(200, 255));
			}
			else
			{
				sprites.at(i)->SetColor(k2d::Color(255, 255));
			}
		}
		else
		{
			if (i % 2 == 0)
			{
				sprites.at(i)->SetColor(k2d::Color(255, 255));
			}
			else
			{
				sprites.at(i)->SetColor(k2d::Color(200, 255));
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
	if (data_points.size() >= max_data_points)
	{
		// Remove the first element
		data_points.erase(data_points.begin());

		// Delete the sprite for the first element
		delete sprites.front();
		sprites.erase(sprites.begin());
	}

	if (data >= max_data_value)
	{
		max_data_value = data;
	}
	data_points.push_back(data);

	k2d::Sprite* new_bar = new k2d::Sprite(glm::vec2(position.x, position.y), size.x / data_points.size(), size.y, 25.0f, glm::vec4(0,0,1,1), k2d::Color(255, 255), bar_texture, sb);
	sprites.push_back(new_bar);
	should_be_gray = !should_be_gray;
	UpdateBarPositions();
}

void UIGraph::AddSprite(k2d::Sprite* sprite)
{
	sprites.push_back(sprite);
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
