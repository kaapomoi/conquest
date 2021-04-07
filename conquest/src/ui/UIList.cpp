#include <ui/UIList.h>

UIList::UIList(std::string name, k2d::vf2d position, k2d::vi2d text_offset, k2d::vf2d size, float depth, float row_height, int max_rows,
	k2d::GLTexture tex, k2d::SpriteBatch* sb, std::map<GLchar, k2d::Character>& _font, float _scale, k2d::Color _color):
	UIBase(name,position, size, depth),
	vector_to_follow(nullptr), background_sprite(nullptr),
	font(_font)
{
	this->needs_update = true;
	this->texture = tex;
	this->max_rows = max_rows;
	this->rows = 0;
	this->sb = sb;
	this->text_offset = text_offset;
	this->row_height = row_height;
	this->text_scale = _scale;

	for (size_t i = 0; i < max_rows; i++)
	{
		text_lines.push_back(new k2d::Text("", font, 0, 0, text_scale, depth + 1.0f, k2d::Color(255), sb));
	}
}

UIList::~UIList()
{
	for (k2d::Text* t : text_lines)
	{
		delete t;
	}
	if (background_sprite)
	{
		delete background_sprite;
	}
}

void UIList::Update(double dt)
{
	if (active)
	{
		if (needs_update)
		{
			UpdatePositions();
		}

		for (k2d::Text* t : text_lines)
		{
			t->Update();
		}

		if (background_sprite)
		{
			background_sprite->Tick();
		}
	}
}

void UIList::UpdatePositions()
{
	float start_pos_y = position.y + size.y * 0.5f - row_height;
	if (vector_to_follow)
	{
		int diff = max_rows - vector_to_follow->size();
		if (diff < 1000000 && diff > -1000000)
		{	
			if (diff >= 0)
			{
				for (size_t i = 0; i < max_rows; i++)
				{
					text_lines.at(i)->SetText("");
				}
				for (size_t i = 0; i < vector_to_follow->size(); i++)
				{
					text_lines.at(i)->SetText(std::to_string(vector_to_follow->at(i)));
					text_lines.at(i)->SetPosition(k2d::vf2d(position.x + text_offset.x, start_pos_y + text_offset.y));
					start_pos_y -= row_height;
				}
			}
			else
			{
				// offset by diff to show the last max_rows of data
				for (size_t i = 0; i < max_rows; i++)
				{
					text_lines.at(i)->SetText(std::to_string(vector_to_follow->at(i + abs(diff))));
					text_lines.at(i)->SetPosition(k2d::vf2d(position.x + text_offset.x, start_pos_y + text_offset.y));
					start_pos_y -= row_height;
				}
			}
		}
	}
}

void UIList::AddBackground(k2d::Color color)
{
	if (!background_sprite)
	{
		background_sprite = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, size.y, depth - 0.1f, glm::vec4(0, 0, 1, 1), color, texture, sb);
	}
	else
	{
		background_sprite->SetColor(color);
	}
}

void UIList::SetVectorToFollow(std::vector<int>* v)
{
	this->vector_to_follow = v;
}

void UIList::UpdateListValues()
{
	needs_update = true;
}
