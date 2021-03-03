#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>

class UIProgressBar : public UIBase
{
public:
	UIProgressBar(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::GLTexture texture, k2d::SpriteBatch* sb);
	~UIProgressBar();

	void Update(double dt);

	void AddProgressValue(int* i);
	void AddProgressValue(float* f);
	void AddProgressValue(double* d);

	void AddTargetValue(int* i);
	void AddTargetValue(float* f);
	void AddTargetValue(double* d);

	void AddText(k2d::Text* text);
	
	void UpdateProgressBarValues();

	void AddBackground(k2d::Color color);

private:
	k2d::Sprite* background_sprite;
	k2d::Sprite* progress_bar_sprite;
	k2d::Text* text;

	k2d::GLTexture texture;
	k2d::SpriteBatch* sb;

	// Progressing values
	int* progress_int;
	float* progress_float;
	double* progress_double;

	int* target_int;
	float* target_float;
	double* target_double;

	bool should_update;
};