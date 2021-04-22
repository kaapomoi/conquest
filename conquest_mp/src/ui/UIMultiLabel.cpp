#include <ui/UIMultiLabel.h>


UIMultiLabel::UIMultiLabel(std::string name, k2d::vi2d position, k2d::vi2d size, float row_height, float scale, float depth, std::map<GLchar, k2d::Character>& _font, k2d::GLTexture tex, k2d::SpriteBatch* _sprite_batch):
	font(_font), UIBase(name, position, size, depth)
{
	this->sb = _sprite_batch;
	this->scale = scale;
	this->background = nullptr;
	this->bg_tex = tex;
	this->row_height = row_height;
}

UIMultiLabel::~UIMultiLabel()
{
	for (k2d::Label* l : label_rows)
	{
		delete l;
	}
	delete background;
}

void UIMultiLabel::AddLabel(std::string name, std::string label_text, int* variable)
{
	k2d::Label* l = new k2d::Label(label_text, font, position.x, position.y, scale, depth, k2d::Color(255), sb);

	l->SetVariable(variable);


	label_rows.push_back(l);
}

void UIMultiLabel::AddBackground(k2d::Color color)
{
	if (!background)
	{
		background = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, size.y, depth - 0.1f, glm::vec4(0, 0, 1, 1), color, bg_tex, sb);
	}
	else
	{
		background->SetColor(color);
	}
}

void UIMultiLabel::Update(double dt)
{
	if (active)
	{

		float start_pos = position.y + (size.y * 0.5f) - row_height;

		for (size_t i = 0; i < label_rows.size(); i++)
		{
			label_rows.at(i)->SetPosition(k2d::vf2d(position.x - (size.x * 0.5f), start_pos));

			label_rows.at(i)->Update();

			start_pos -= row_height;
		}

		if (background)
		{
			background->Tick();
		}
	}

}

void UIMultiLabel::SetPosition(k2d::vf2d pos)
{
	if (background)
	{
		background->SetPosition(pos);
	}
	for (size_t i = 0; i < label_rows.size(); i++)
	{
		label_rows.at(i)->SetPosition(pos);
	}
}
