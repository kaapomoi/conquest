#include <core/UIUnitCard.h>

UIUnitCard::UIUnitCard(std::string name, k2d::vi2d position, float width, float height, k2d::Sprite* sprite, Unit* unit, k2d::SpriteBatch* sb, std::map<GLchar, k2d::Character>& font)
	: UIElement(name, position, sprite, 0)
{
	// Create the ui elements
	this->width = width;
	this->height = height;
	this->sb = sb;
	this->unit = unit;
	this->font = font;
	this->position = position;

	name_text = new k2d::Text("Name", this->font, 0, 0,		0.10f, 22.0f, k2d::Color(255), this->sb);
	health_text = new k2d::Text("Health", this->font, 0, 0, 0.10f, 22.0f, k2d::Color(255), this->sb);
	mana_text = new k2d::Text("Mana", this->font, 0, 0,		0.10f, 22.0f, k2d::Color(255), this->sb);
	ad_text = new k2d::Text("AD", this->font, 0, 0,			0.10f, 22.0f, k2d::Color(255), this->sb);
	as_text = new k2d::Text("AS", this->font, 0, 0,			0.10f, 22.0f, k2d::Color(255), this->sb);
	state_text = new k2d::Text("STATE", this->font, 0, 0,	0.10f, 22.0f, k2d::Color(255), this->sb);

	// Reposition the UI elements
	SetPosition(position);
}

UIUnitCard::~UIUnitCard()
{
	delete name_text;
	delete health_text;
	delete mana_text;
	delete ad_text;
	delete as_text;
	delete state_text;
}


// Recursively updates this and children
void UIUnitCard::Update(double dt)
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

		if (unit != nullptr)
		{
			name_text->SetText(unit->GetName());
			health_text->SetText("HP: " + std::to_string((int)unit->GetHealth()) + "/" + std::to_string((int)unit->GetMaxHealth()));
			mana_text->SetText("MP: " + std::to_string((int)unit->GetMana()) + "/" + std::to_string((int)unit->GetMaxMana()));
			ad_text->SetText("AD: " + std::to_string(unit->GetAttackDamage()));
			as_text->SetText("AS: " + std::to_string(unit->GetAttackSpeed()));
			state_text->SetText("STATE: " +std::to_string((int)unit->GetState()));
		}

		name_text->Update();
		health_text->Update();
		mana_text->Update();
		ad_text->Update();
		as_text->Update();
		state_text->Update();

		if (unit_sprite != nullptr)
		{
			unit_sprite->Tick();
		}

		if (child != nullptr)
		{
			this->child->Update(dt);
		}
	}
	
}

void UIUnitCard::SetPosition(k2d::vf2d new_pos)
{
	this->position = new_pos;
	name_text->SetPosition(new_pos - k2d::vf2d(46.f, -40.f));
	health_text->SetPosition(new_pos - k2d::vf2d(46.f, -20.f));
	mana_text->SetPosition(new_pos - k2d::vf2d(46.f, 0.f));
	ad_text->SetPosition(new_pos - k2d::vf2d(46.f, 20.f));
	as_text->SetPosition(new_pos - k2d::vf2d(46.f, 40.f));
	state_text->SetPosition(new_pos - k2d::vf2d(46.f, 60.f));

	UIElement::SetPosition(new_pos);
}

void UIUnitCard::SetUnitSprite(k2d::Sprite* sprite)
{
	this->unit_sprite = sprite;
}

void UIUnitCard::SetUnit(Unit* u)
{
	this->unit = u;
}

void UIUnitCard::SetUnitSpritePosition(k2d::vf2d new_pos)
{
	if (unit_sprite != nullptr)
	{
		unit_sprite->SetPosition(glm::vec2(position.x + new_pos.x, position.y + new_pos.y));
	}
}
