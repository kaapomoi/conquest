#include <ui/UIClickableGraph.h>

UIClickableGraph::UIClickableGraph(std::string name, k2d::vi2d position, k2d::vi2d size, float depth, int max_data_points, float max_data_value, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb):
	UIGraph(name, position, size, depth, max_data_points, max_data_value, bar_tex, sb)
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

void UIClickableGraph::OnClick(k2d::vf2d relative_position)
{
	if (!data_points->empty())
	{

		float bar_width = (float) size.x / (float) max_data_points;

		int resolution = 1;

		if (bar_width < 1.0f)
		{
			resolution = ceil(1 / bar_width);
		}

		float percentage_of_x_size = (float) relative_position.x / (float) size.x;

		float click_height = (float)relative_position.y /(float)size.y;

		int index = size.x * percentage_of_x_size / bar_width;

		if (index >= max_data_points)
		{
			index = max_data_points - 1;
		}

		// modify each bar to the right of the resolution threshhold
		for (size_t	i = index; i < index + resolution && i < max_data_points; i++)
		{
			data_points->at(i) = click_height * max_data_value;
		}
		UpdateBarPositions();

	// Call callbacks
	}
	UIClickable::OnClick(relative_position);
}
