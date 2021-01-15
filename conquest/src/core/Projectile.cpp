#include <core/Projectile.h>
#include <core/Unit.h>

Projectile::Projectile(k2d::vi2d position, float width, float height, k2d::Sprite* sprite,
	float attack_damage, float magic_damage, float true_damage, float movement_speed, Unit* target, Unit* sender)
	: GameObject(position, width, height, sprite)
{

	this->movement_speed = movement_speed;
	this->attack_damage = attack_damage;
	this->magic_damage = magic_damage;
	this->true_damage = true_damage;
	this->target = target;
	this->sender = sender;
}

Projectile::~Projectile()
{

}

bool Projectile::Update(double deltatime)
{
	if (target == nullptr)
	{
		return true;
	}
	else
	{
		if (!target->GetIsAlive())
		{
			target = nullptr;
			sender->SetAttackTarget(nullptr);
		}
	}

	k2d::vf2d dir = target->GetPosition() - this->position;
	dir = dir.norm();
	Move(dir * movement_speed * deltatime);

	if ((target->GetPosition() - this->position).mag2() < 32.0f)
	{
		if (target->TakeDamage(attack_damage, magic_damage, true_damage))
		{
			// if target dies
			sender->SetAttackTarget(nullptr);
		}
		return true;
	}

	// Draw the sprite
	if (sprite != nullptr)
	{
		sprite->SetPosition(glm::vec2(position.x, position.y));
		sprite->Tick();
	}
	return false;
}


