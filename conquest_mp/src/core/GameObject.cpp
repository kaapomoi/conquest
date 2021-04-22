#include <core/GameObject.h>

GameObject::GameObject(k2d::vi2d position, float width, float height, k2d::Sprite* sprite)
{
	this->position = position;
	this->width = width;
	this->height = height;
	this->sprite = sprite;
	sprite->SetPosition(glm::vec2(position.x, position.y));
}

GameObject::~GameObject()
{
	delete sprite;
}

bool GameObject::Update(double deltatime)
{
	// Update position and other variables
	sprite->SetPosition(glm::vec2(position.x, position.y));

	// Draw the sprite
	if (sprite != nullptr)
	{
		sprite->Tick();
	}
	return false;
}


