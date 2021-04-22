#pragma once
#include <core/Engine.h>
#include <core/AStarNode.h>

class GameObject
{
public:
	GameObject(k2d::vi2d position, float width, float height, k2d::Sprite* sprite);
	virtual ~GameObject();

	/// <summary>
	/// Updates basic gameobject
	/// </summary>
	/// <returns>Bool whether this should be removed from a vector</returns>
	virtual bool Update(double deltatime);

	void Move(k2d::vi2d amount) { position += amount; }
	void SetPosition(k2d::vi2d pos) { position = pos; }
	k2d::vi2d GetPosition() { return position; }
	k2d::Sprite* GetSprite() { return this->sprite; }

protected:
	k2d::vi2d position;
	k2d::Sprite* sprite;

	float width;
	float height;
};