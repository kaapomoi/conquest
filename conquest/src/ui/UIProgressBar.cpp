#include <ui/UIProgressBar.h>


UIProgressBar::UIProgressBar(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::GLTexture texture, k2d::SpriteBatch* sb):
	UIBase(name), progress_int(nullptr), progress_float(nullptr), progress_double(nullptr),target_int(nullptr), target_float(nullptr), target_double(nullptr)
{
	this->position = position;
	this->size = size;
	this->depth = depth;
	this->texture = texture;
	this->sb = sb;
	this->text = nullptr;
	this->should_update = true;
	this->active = true;
	this->progress_bar_sprite = new k2d::Sprite(glm::vec2(position.x, position.y), size.x, size.y, depth, glm::vec4(0,0,1,1), k2d::Color(255), texture, sb);
}

UIProgressBar::~UIProgressBar()
{

}

void UIProgressBar::Update(double dt)
{
	if (should_update)
	{
		float value = 0.0f;
		if (progress_int)
		{
			value = *progress_int;
		}
		else if (progress_float)
		{
			value = *progress_float;
		}
		else if (progress_double)
		{
			value = *progress_double;
		}
		
		float target = 0.0f;
		if (target_int)
		{
			target = *target_int;
		}
		else if (target_float)
		{
			target = *target_float;
		}
		else if (target_double)
		{
			target = *target_double;
		}

		float width_percent = value / target;
		k2d::clamp(width_percent, 0.0f, 1.0f);
		float width = width_percent * size.x;

		progress_bar_sprite->SetWidth(width);
		progress_bar_sprite->SetPosition(glm::vec2(position.x + width / 2, position.y));

		should_update = false;
	}

	progress_bar_sprite->Tick();

	if (text)
	{
		text->Update();
	}

}

void UIProgressBar::AddProgressValue(int* i)
{
	progress_int = i;
}

void UIProgressBar::AddProgressValue(float* f)
{
	progress_float = f;
}

void UIProgressBar::AddProgressValue(double* d)
{
	progress_double = d;
}

void UIProgressBar::AddTargetValue(int* i)
{
	target_int = i;
}

void UIProgressBar::AddTargetValue(float* f)
{
	target_float = f;
}

void UIProgressBar::AddTargetValue(double* d)
{
	target_double = d;
}

void UIProgressBar::AddText(k2d::Text* text)
{
	this->text = text;
}

void UIProgressBar::SetActive(bool a)
{
	active = a;
}

void UIProgressBar::UpdateProgressBarValues()
{
	should_update = true;
}
