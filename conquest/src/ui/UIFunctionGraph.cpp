#include <ui/UIFunctionGraph.h>

UIFunctionGraph::UIFunctionGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points,  k2d::GLTexture bar_tex, k2d::SpriteBatch* sb, the_function f) :
	UIGraph(name, position, size, max_data_points, find_max(0, max_data_points, f), bar_tex, sb), func_pointer(f)
{
	// Add empty datapoints to underlying graph
	for (size_t i = 0; i < max_data_points; i++)
	{
		UIGraph::AddDataPoint(0);
	}

	UpdateGraphValues();
}

UIFunctionGraph::~UIFunctionGraph()
{
	//Delete own stuff, then bases stuff


	UIGraph::~UIGraph();
}

void UIFunctionGraph::Update(double dt)
{


	// Update base
	UIGraph::Update(dt);
}

void UIFunctionGraph::SetFunctionPointer(the_function f)
{
	func_pointer = f;
}

void UIFunctionGraph::UpdateGraphValues()
{
	if (func_pointer)
	{
		float offset = 0.0f;
		float width = (float)size.x / (float)max_data_points;

		// start position = left
		float start_x = position.x - size.x * 0.5f + width / 2;
		for (size_t i = 0; i < data_points.size(); i++)
		{
			// calculate the value for the position i ( x )
			data_points.at(i) = func_pointer(i);
			float height = data_points.at(i) / max_data_value * size.y;
			
			bar_sprites.at(i)->SetWidth(width);
			bar_sprites.at(i)->SetPosition(glm::vec2(start_x + offset, position.y + height * 0.5f));
			bar_sprites.at(i)->SetHeight(height);
			offset += width;
			if (should_be_gray && data_points.size() >= max_data_points)
			{
				if (i % 2 == 0)
				{
					bar_sprites.at(i)->SetColor(k2d::Color(200, 255));
				}
				else
				{
					bar_sprites.at(i)->SetColor(k2d::Color(255, 255));
				}
			}
			else
			{
				if (i % 2 == 0)
				{
					bar_sprites.at(i)->SetColor(k2d::Color(255, 255));
				}
				else
				{
					bar_sprites.at(i)->SetColor(k2d::Color(200, 255));
				}
			}
		}
	}
	else
	{
		k2d::KUSI_DEBUG("!!!!!No function passed to the graph, not updating values!!!!!\n");
	}
}
