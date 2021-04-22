#pragma once

#include <core/Engine.h>
#include <ui/UIBase.h>

class UIRectangle : public UIBase
{
public:
	UIRectangle(std::string name, k2d::vf2d position, k2d::vf2d size, float depth, k2d::Sprite* sprite);
	virtual ~UIRectangle();

	void Update(double dt)override;

	void SetColor(k2d::Color c);

	virtual void SetDepth(float d)override;
	virtual void SetPosition(k2d::vf2d pos)override;
	virtual void SetSize(k2d::vf2d size)override;



private:
	k2d::Sprite* sprite;

};