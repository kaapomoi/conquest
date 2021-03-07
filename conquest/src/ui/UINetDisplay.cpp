#include <ui/UINetDisplay.h>

UINetDisplay::UINetDisplay(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::vf2d node_size): 
	UIBase(name, position, size, depth)
{
	this->node_size = node_size;
	this->net = nullptr;
}

UINetDisplay::~UINetDisplay()
{

}

void UINetDisplay::Update(double dt)
{
	UpdatePositions();
	for (k2d::Sprite* s : node_sprites)
	{
		s->Tick();
	}
	for (k2d::Sprite* s : weight_sprites)
	{
		s->Tick();
	}
}

void UINetDisplay::SetNeuralNetPtr(NeuralNet* nn)
{
	this->net = nn;

}

void UINetDisplay::UpdatePositions()
{

}
