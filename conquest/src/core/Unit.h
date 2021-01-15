#pragma once
#include <core/Engine.h>
#include <core/GameObject.h>
#include <core/Projectile.h>

enum class SPELLTARGETTYPE
{
	NONE = 0,
	CLOSEST,
	FARTHEST,
	MOST_AD,
	MOST_AP,
	MOST_AS,
	MOST_HP,
	MOST_PHP,
	MOST_MANA,
	MOST_PMANA,
	LEAST_AD,
	LEAST_AP,
	LEAST_AS,
	LEAST_HP,
	LEAST_PHP,
	LEAST_MANA,
	LEAST_PMANA,
	RANDOM_AOE,
	RANDOM_TARGET,
	COUNT
};

enum class STATE
{
	IDLE = 0,
	SEARCH,
	MOVING,
	WINDUP,
	SETTLINGINTOSQUARE,
	PATHAGAIN,
};

class Unit : public GameObject
{
public:
	Unit(k2d::vi2d position, float width, float height, k2d::Sprite* sprite,
		float health, float max_health, float mana, float max_mana, float attack_damage,
		float base_attack_speed, float bonus_attack_speed, float windup_percent, float windup_modifier,
		float movement_speed, int attack_range, int team, std::string name,
		k2d::Sprite* hp_bar_sprite, k2d::Sprite* mana_bar_sprite, k2d::Sprite* stat_bar_container);
	virtual ~Unit() override;

	virtual bool Update(double deltatime) override;

	void SetPathToTargetSquare(std::vector<k2d::vi2d> vec);
	void SetHealth(float health) { this->health = health; }
	//TODO: Calculate real damages !
	bool TakeDamage(float pd, float md, float td);

	void SetAtkDamage(float atk_dmg) { this->attack_damage = atk_dmg; }
	void SetAtkRange(int range) { this->attack_range = range; }
	void SetState(STATE state) { this->current_state = state; }
	void SetTargetSquare(k2d::vi2d target_square) { this->target_square = target_square; }
	void SetAttackTarget(Unit* u) { this->attack_target = u; }
	void SetOnCooldown(bool cd) { this->on_cooldown = cd; }
	void SetWindUpTimer(float time) { this->windup_timer = time; }
	void SetWindUpTime(float time) { this->windup_time = time; }
	void SetCurrentSquare(k2d::vi2d s) { this->current_square = s; }
	void SetOriginalTargetSquare(k2d::vi2d s) { this->original_square_of_target = s; }

	void AddProjectile(Projectile* p) { projectiles.push_back(p); }

	void ClearPath();

	k2d::vi2d GetFirstPathSquare() { if (path_to_target_square.size() > 0) return path_to_target_square.at(0); }
	void PopFirstPathSquare() { if (path_to_target_square.size() > 0) path_to_target_square.pop_front(); }

	std::string GetName() { return this->name; }
	int GetAtkRange() { return this->attack_range; }
	int GetTeam() { return this->team; }
	float GetAttackDamage() { return this->attack_damage; }
	float GetAttackSpeed() { return base_attack_speed + bonus_attack_speed; }
	float GetMovementSpeed() { return this->movement_speed; }
	float GetMaxHealth() { return this->max_health; }
	float GetHealth() { return this->health; }
	float GetMaxMana() { return this->max_mana; }
	float GetMana() { return this->mana; }
	Unit* GetAttackTarget() { return this->attack_target; }
	STATE GetState() { return this->current_state; }
	k2d::vi2d GetTargetSquare() { return this->target_square; }
	bool GetIsAlive() { return this->is_alive; }
	bool GetOnCooldown() { return this->on_cooldown; }
	float GetWindUpTimer() { return this->windup_timer; }
	float GetWindUpTime() { return this->windup_time; }
	k2d::vi2d GetCurrentSquare() { return this->current_square; }
	k2d::vi2d GetOriginalTargetSquare() { return this->original_square_of_target; }

protected:
	std::string name;
	float health;
	float max_health;
	float attack_damage;
	float mana;
	float max_mana;
	float base_attack_speed;
	float bonus_attack_speed;
	int attack_range; // Range in squares
	float movement_speed;

	bool on_cooldown;
	bool in_range;

	float cooldown_timer;

	float windup_timer;
	float windup_percent;
	float windup_modifier;
	float windup_time;

	bool is_alive;

	int team;

	std::vector<Projectile*> projectiles;

	Unit* attack_target;

	STATE current_state;

	std::deque<k2d::vi2d> path_to_target_square;
	k2d::vi2d target_square;
	k2d::vi2d original_square_of_target;
	k2d::vi2d current_square;

	k2d::Sprite* health_bar_sprite;
	k2d::Sprite* stat_bar_container;
	k2d::Sprite* mana_bar_sprite;
};