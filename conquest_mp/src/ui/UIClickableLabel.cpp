#include <ui/UIClickableLabel.h>

UIClickableLabel::UIClickableLabel(std::string name, std::string label_text, k2d::vi2d position, k2d::vi2d text_offset, k2d::vi2d size, k2d::GLTexture tex, k2d::SpriteBatch* sb,
	std::map<GLchar, k2d::Character>& _font, float _scale, float _depth, k2d::Color _color):
	 UIBase(name, position, size, depth)
{
	this->position = position;
	this->size = size;
	this->bar_texture = tex;
	this->sb = sb;
	this->text_offset = text_offset;
	this->active = true;
	this->is_hit = false;
	this->modifiable = false;
	this->depth = depth;
	this->background = nullptr;
	this->variable_multiplier = 1;
	// TODO : fix position to accomodate offsets
	label = new k2d::Label(label_text, _font, position.x, position.y, _scale, _depth, _color, sb);
	this->SetTextOffset(text_offset);
}

UIClickableLabel::~UIClickableLabel()
{
	delete label;
	delete background;
}

// Recursively updates this and children
void UIClickableLabel::Update(double dt)
{
	if (active)
	{
		if (background)
		{
			background->Tick();
		}
		if (label)
		{
			label->Update();
		}
	}
}

void UIClickableLabel::SetPosition(k2d::vf2d new_pos)
{
	if (background)
	{
		background->SetPosition(new_pos);
	}
	if (label)
	{
		label->SetPosition(new_pos);
	}
	UIBase::SetPosition(new_pos);
}

void UIClickableLabel::SetIsHit(bool is_hit)
{
	this->is_hit = is_hit;
}

void UIClickableLabel::SetVariable(int* i)
{
	label->SetVariable(i);
}

void UIClickableLabel::SetVariable(float* f)
{
	label->SetVariable(f);
}

void UIClickableLabel::SetVariable(double* d)
{
	label->SetVariable(d);
}

void UIClickableLabel::SetBaseMultiplier(int i)
{
	label->SetBaseMultiplier(i);
}

void UIClickableLabel::SetBaseMultiplier(float f)
{
	label->SetBaseMultiplier(f);
}

void UIClickableLabel::SetBaseMultiplier(double d)
{
	label->SetBaseMultiplier(d);
}

void UIClickableLabel::SetPrintPrecision(int decimal_points)
{
	label->SetPrecision(decimal_points);
}

void UIClickableLabel::SetModifiable(bool m)
{
	modifiable = m;
}

void UIClickableLabel::SetVariableMultiplier(int mul)
{
	label->SetVariableMultiplier(mul);
}

void UIClickableLabel::SetPrettyPrintFunc(pretty_print_func func)
{
	label->SetPrettyPrintFunc(func);
}

void UIClickableLabel::SetBackground(k2d::Color bg_color)
{
	if (!background)
	{
		background = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, size.y, label->GetDepth() - 0.1f, glm::vec4(0, 0, 1, 1), bg_color, bar_texture, sb);
	}
	else
	{
		background->SetColor(bg_color);
	}
}

void UIClickableLabel::OnClick(k2d::vf2d relative_hit_pos)
{
	if (modifiable)
	{
		if (relative_hit_pos.x >= (size.x * 0.5f))
		{
			// We hit the right side 
			label->RaiseVariableValue();
		}
		else
		{
			// We hit the left side
			label->LowerVariableValue();
		}
	}
	UIClickable::OnClick(relative_hit_pos);
}