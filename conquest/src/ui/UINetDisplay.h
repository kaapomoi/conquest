#pragma once

#include <core/Application.h>
#include <ui/UIBase.h>
#include <ui/UIClickable.h>
#include <neuralnet/NeuralNet.h>

class UINetDisplay : public UIBase, public UIClickable
{
public:
	UINetDisplay(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::GLTexture texture, k2d::SpriteBatch* sb, k2d::Application* app);
	~UINetDisplay();

	void Update(double dt) override;

	void SetNeuralNetPtr(NeuralNet* nn);

	void UpdateAlphas();

	void AddBackground(k2d::Color color);

	void OnClick() override;

	void ToggleWeightsOnlyMode();

private:
	NeuralNet*					net;
	k2d::Application*			app;
	
	k2d::GLTexture				texture;
	k2d::SpriteBatch*			sb;

	bool						weights_only_mode;

	k2d::vf2d					node_size;
	k2d::vf2d					weight_size;

	k2d::Sprite*				background_sprite;

	std::vector<int>			topology;
	std::vector<k2d::Sprite*>	node_sprites;
	std::vector<k2d::Sprite*>	weight_sprites;
	std::vector<std::vector<k2d::vf2d>>		node_positions;

};