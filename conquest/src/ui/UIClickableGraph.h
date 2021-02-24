#pragma once

#include <ui/UIGraph.h>

class UIClickableGraph : public UIGraph
{
public:
	UIClickableGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points, float max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb);
	virtual ~UIClickableGraph();

	void Update(double dt);

	void OnHit(k2d::vf2d relative_position) override;

private:


};