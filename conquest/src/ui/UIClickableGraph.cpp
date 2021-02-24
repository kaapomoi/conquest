#include <ui/UIClickableGraph.h>

UIClickableGraph::UIClickableGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points, float max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb):
	UIGraph(name, position, size, max_data_points, max_data_value, bar_tex, sb)
{
	
}

UIClickableGraph::~UIClickableGraph()
{


	UIGraph::~UIGraph();
}

void UIClickableGraph::Update(double dt)
{


	UIGraph::Update(dt);
}

void UIClickableGraph::OnHit(k2d::vf2d relative_position)
{
	float bar_width = (float) size.x / (float) max_data_points;
	float percentage_of_x_size = (float) relative_position.x / (float) size.x;

	float click_height = (float)relative_position.y /(float)size.y;

	int index = size.x * percentage_of_x_size / bar_width;

	if (index >= max_data_points)
	{
		index = max_data_points - 1;
	}

	// Value is between 0.f and 1.0f
	/*float* f = data_points->data();
	f += index;
	*f = click_height;*/
	data_points->at(index) = click_height * max_data_value;
	UpdateBarPositions();

	// Call callbacks
	UIClickable::OnClick();
}
