#include <core/UIElement.h>

UIElement::UIElement(std::string name, k2d::vi2d position, k2d::Sprite* sprite, k2d::Text* text)
{
	this->name = name;
	this->sprite = sprite;
	this->text = text;
	this->child = nullptr;
	this->position = position;
	this->active = true;
	glm::vec2 p;
	if (sprite!= nullptr)
	{
		sprite->SetPosition(glm::vec3(position.x, position.y, 0.0f));
		p = sprite->GetPosition();
	}

	text_pos_offset = 0;
	if (text != nullptr)
	{
		text->SetPosition(k2d::vf2d(p.x, p.y) + text_pos_offset);
	}
}

UIElement::~UIElement()
{
	delete sprite;
	delete text;
	DestroyChildren();
}

void UIElement::AddChild(UIElement* child)
{
	if (this->child == nullptr)
	{
		this->child = child;
	}
	else
	{
		this->child->AddChild(child);
	}

}

void UIElement::DestroyChildren()
{
	while (child)
	{
		UIElement* temp = child;
		child = temp->GetChild();
		delete temp;
	}
}

// Recursively updates this and children
void UIElement::Update(double dt)
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

		if (child != nullptr)
		{
			this->child->Update(dt);
		}
	}
}

void UIElement::SetPosition(k2d::vf2d new_pos)
{
	if (sprite != nullptr)
	{
		sprite->SetPosition(glm::vec3(new_pos.x, new_pos.y, 0.0f));
		glm::vec2 p = sprite->GetPosition();
		if (text != nullptr)
		{
			text->SetPosition(k2d::vf2d(p.x, p.y) + text_pos_offset);
		}
	}
}

void UIElement::SetTextOffset(k2d::vf2d offset)
{
	text_pos_offset = offset;
	glm::vec2 p = sprite->GetPosition();
	if (text != nullptr)
	{
		text->SetPosition(k2d::vf2d(p.x, p.y) + text_pos_offset);
	}
}

void UIElement::SetSprite(k2d::Sprite* sprite)
{
	this->sprite = sprite;
}

void UIElement::SetText(k2d::Text* text)
{
	if (text != nullptr)
	{
		this->text = text;
	}
}

void UIElement::SetName(std::string name)
{
	this->name = name;
}

void UIElement::SetActualText(std::string new_text)
{
	if (text != nullptr)
	{
		text->SetText(new_text);
	}
}

void UIElement::SetIsActive(bool a)
{
	while (this->child)
	{
		child->SetIsActive(a);
	}
	this->active = a;
}
