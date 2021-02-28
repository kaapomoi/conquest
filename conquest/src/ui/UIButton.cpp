#include <ui/UIButton.h>


UIButton::UIButton(std::string name, k2d::vi2d position, k2d::vi2d size, k2d::Sprite* sprite, k2d::Text* text):
	UIBase(name)
{
	this->sprite = sprite;
	this->text = text;

	this->position = position;
	this->size = size;
	this->active = true;

	if (sprite != nullptr)
	{
		sprite->SetPosition(glm::vec3(position.x, position.y, 0.0f));
		sprite->SetWidth(size.x);
		sprite->SetHeight(size.y);
	}

	text_pos_offset = 0;
	if (text != nullptr)
	{
		text->SetPosition(position + text_pos_offset);
	}
}

UIButton::~UIButton()
{
	delete sprite;
	delete text;
}

void UIButton::Update(double dt)
{
	if (active)
	{
		if (sprite != nullptr)
		{
			sprite->Tick();
		}

		if (text != nullptr)
		{
			text->Update();
		}
	}
}

void UIButton::SetPosition(k2d::vi2d new_pos)
{
	position = new_pos;
	if (sprite != nullptr)
	{
		sprite->SetPosition(glm::vec3(new_pos.x, new_pos.y, 0.0f));
		glm::vec2 p = sprite->GetPosition();	
	}
	if (text != nullptr)
	{
		text->SetPosition(position + text_pos_offset);
	}
}

void UIButton::SetTextOffset(k2d::vi2d offset)
{
	text_pos_offset = offset;
	if (text != nullptr)
	{
		text->SetPosition(position + text_pos_offset);
	}
}

void UIButton::SetSprite(k2d::Sprite* sprite)
{
	// Delete the old sprite
	if (this->sprite)
	{
		delete this->sprite;
	}
	// Set the new sprite
	this->sprite = sprite;
}

void UIButton::SetText(k2d::Text* text)
{
	// delete the old text
	if (this->text)
	{
		delete this->text;
	}
	// Set the new text
	this->text = text;
}

void UIButton::SetActualText(std::string new_text)
{
	if (text)
	{
		text->SetText(new_text);
	}
}

void UIButton::SetIsActive(bool a)
{
	active = a;
}

void UIButton::OnClick()
{
	UIClickable::OnClick();
}

void UIButton::OnClick(k2d::vf2d rel_click_pos)
{
	UIClickable::OnClick(rel_click_pos);
}
