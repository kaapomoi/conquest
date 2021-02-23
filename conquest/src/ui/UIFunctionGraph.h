#pragma once

#include <ui/UIGraph.h>

typedef float (*the_function)(float);

namespace 
{
	float find_max(int first, int last, the_function f_pointer)
	{
		// Loop from first to one before last, record highest value
		float max = -9999999.0f;
		for (size_t i = first; i < last; i++)
		{
			// send the iterative number to the function, collect the output and compare to current max
			if (max < f_pointer(i))
			{
				max = f_pointer(i);
			}
		}
		return max;
	}
}

class UIFunctionGraph : public UIGraph
{
public:
	UIFunctionGraph(std::string name, k2d::vi2d position, k2d::vi2d size, int max_data_points, k2d::GLTexture bar_tex, k2d::SpriteBatch* sb, the_function f);
	virtual ~UIFunctionGraph();

	void Update(double dt);

	void SetFunctionPointer(the_function f);


private:
	void UpdateGraphValues();


	the_function func_pointer;
};