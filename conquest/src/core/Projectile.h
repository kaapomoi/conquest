#pragma once
#include <core/Engine.h>
#include <core/GameObject.h>
class Unit;

class Projectile : public GameObject
{
public:
	Projectile(k2d::vi2d position, float width, float height, k2d::Sprite* sprite,
		float attack_damage, float magic_damage, float true_damage, float movement_speed, Unit* target, Unit* sender);
	virtual ~Projectile() override;

	virtual bool Update(double deltatime) override;

	void SetAtkDamage(float atk_dmg) { this->attack_damage = atk_dmg; }
	void SetAttackTarget(Unit* u) { this->target = u; }

	float GetMovementSpeed() { return this->movement_speed; }
	Unit* GetAttackTarget() { return this->target; }

protected:
	float attack_damage;
	float magic_damage;
	float true_damage;
	float movement_speed;

	Unit* target;
	Unit* sender;

};