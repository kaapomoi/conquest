#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>
#include <neuralnet/NeuralNet.h>

class UINetDisplay : public UIBase
{
public:
	UINetDisplay(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::vf2d node_size);
	~UINetDisplay();

	void Update(double dt) override;

	void SetNeuralNetPtr(NeuralNet* nn);

	void UpdatePositions();

private:
	NeuralNet*					net;
	
	k2d::vf2d node_size;
	std::vector<k2d::Sprite*>	node_sprites;
	std::vector<k2d::Sprite*>	weight_sprites;
	
};