#include <core/Unit.h>

Unit::Unit(k2d::vi2d position, float width, float height, k2d::Sprite* sprite,
	float health, float max_health, float mana, float max_mana, float attack_damage,
	float base_attack_speed, float bonus_attack_speed, float windup_percent, float windup_modifier,
	float movement_speed, int attack_range, int team, std::string name,
	k2d::Sprite* hp_bar_sprite, k2d::Sprite* mana_bar_sprite, k2d::Sprite* stat_bar_container)
	: GameObject(position, width, height, sprite)
{
	this->name = name;
	this->health = health;
	this->max_health = max_health;
	this->mana = mana;
	this->max_mana = max_mana;
	this->attack_damage = attack_damage;
	this->base_attack_speed = base_attack_speed;
	this->bonus_attack_speed = bonus_attack_speed;
	this->windup_percent = windup_percent;
	this->windup_modifier = windup_modifier;
	this->attack_range = attack_range;
	this->movement_speed = movement_speed;
	this->team = team;
	this->stat_bar_container = stat_bar_container;
	this->health_bar_sprite = hp_bar_sprite;
	this->mana_bar_sprite = mana_bar_sprite;
	this->in_range = false;
	this->on_cooldown = false;
	this->is_alive = true;
	this->current_state = STATE::SEARCH;
	this->attack_target = nullptr;


	this->windup_time = (1.0f / (this->base_attack_speed + this->bonus_attack_speed)) * this->windup_percent * (1 + this->bonus_attack_speed * (1 - this->windup_modifier));

}

Unit::~Unit()
{
	delete health_bar_sprite;
	delete mana_bar_sprite;
	delete stat_bar_container;
}

bool Unit::Update(double deltatime)
{
	// Iterate and update, maybe delete projs if necessary
	if (projectiles.size() > 0)
	{
		auto ite = projectiles.begin();
		while (ite != projectiles.end())
		{
			if ((*ite)->Update(deltatime))
			{
				delete* ite;
				ite = projectiles.erase(ite);
			}
			else
			{
				ite++;
			}
		}
	}

	if (health <= 0.0f)
	{
		is_alive = false;
		return true;
	}

	if (attack_target != nullptr)
	{
		if (!attack_target->GetIsAlive())
		{
			attack_target = nullptr;
		}
	}
	
	if (on_cooldown)
	{
		cooldown_timer += deltatime;
		if (cooldown_timer >= (1 / (base_attack_speed + bonus_attack_speed)))
		{
			on_cooldown = false;
			cooldown_timer = 0.0f;
		}
	}



	// Health bar
	if (health_bar_sprite != nullptr)
	{
		health_bar_sprite->SetWidth(width * health / max_health);
		health_bar_sprite->SetPosition(glm::vec2(position.x - (width / 2 - health_bar_sprite->GetDimensions().x / 2), position.y));
		health_bar_sprite->Tick();
	}

	// Mana bar
	if (mana_bar_sprite != nullptr)
	{
		mana_bar_sprite->SetWidth(width * mana / max_mana);
		mana_bar_sprite->SetPosition(glm::vec2(position.x - (width / 2 - mana_bar_sprite->GetDimensions().x / 2), position.y));
		mana_bar_sprite->Tick();
	}

	// Bar background / container
	if (stat_bar_container != nullptr)
	{
		stat_bar_container->SetPosition(glm::vec2(position.x, position.y));
		stat_bar_container->Tick();
	}

	// Draw the sprite
	if (sprite != nullptr)
	{
		sprite->SetPosition(glm::vec2(position.x, position.y));
		sprite->Tick();
	}

	
	

	return false;
}

void Unit::SetPathToTargetSquare(std::vector<k2d::vi2d> vec)
{
	for (k2d::vi2d p : vec)
	{
		path_to_target_square.push_front(p);
	}
}

bool Unit::TakeDamage(float pd, float md, float td)
{
	health -= pd + md + td; 
	if (health <= 0.0f)
	{
		return true;
	}
}

void Unit::ClearPath()
{
	path_to_target_square.clear();
}


