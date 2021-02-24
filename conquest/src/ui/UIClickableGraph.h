#pragma once

#include <ui/UIGraph.h>
#include <ui/UIClickable.h>

class UIClickableGraph : public UIGraph, public UIClickable
{
public:
	UIClickableGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points, float max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb);
	virtual ~UIClickableGraph();

	void Update(double dt);

	void OnClick(k2d::vf2d relative_position) override;

private:


};