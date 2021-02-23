#include <graphics/Label.h>

namespace k2d
{

	Label::Label(std::string _text, std::map<GLchar, Character>& _characters, float _x, float _y,
		float _scale, float _depth, Color _color, SpriteBatch* _sprite_batch) :
		Text(_text, _characters, _x, _y, _scale, _depth, _color, _sprite_batch), label_text(_text), variable_multiplier(1),
		int_to_watch(nullptr), float_to_watch(nullptr), double_to_watch(nullptr), print_func(nullptr)
	{
		base_mul_int = 1;
		base_mul_float = 0.01f;
		base_mul_double = 0.01;
	}

	Label::~Label()
	{

	}

	void Label::Update()
	{

		if (print_func)
		{
			if (int_to_watch)
			{
				value = k2d::to_string_p(print_func(*int_to_watch), print_precision);
			}
			else if (float_to_watch)
			{
				value = k2d::to_string_p(print_func(*float_to_watch), print_precision);
			}
			else if (double_to_watch)
			{
				value = k2d::to_string_p(print_func(*double_to_watch), print_precision);
			}
		}
		else
		{
			if (int_to_watch)
			{
				value = std::to_string(*int_to_watch);
			}
			else if (float_to_watch)
			{
				value = std::to_string(*float_to_watch);
			}
			else if (double_to_watch)
			{
				value = std::to_string(*double_to_watch);
			}
		}
		

		// un-point the pointer to get the variables value
		
		// Combine the value and label text
		Text::SetText(label_text + value);
		
		// Iterate through all characters

		fake_x = x;
		fake_y = y;

		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = characters[*c];

			float xpos = fake_x + ch.bearing.x * scale;
			float ypos = fake_y - (ch.size.y - ch.bearing.y) * scale;

			float w = ch.size.x * scale;
			float h = ch.size.y * scale;

			glm::vec4 dest_rect(xpos, ypos, w, h);
			glm::vec4 uv_rect(0.0f, 0.0f, 1.0f, 1.0f);

			// DEBUG
			//std::cout << ch.size.x << ", " << ch.size.y << "\n";

			sprite_batch->Draw(dest_rect, uv_rect, ch.texture_id, color, depth);

			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			fake_x += (ch.advance >> 6)* scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}

	}

	void Label::SetVariable(int* i)
	{
		int_to_watch = i;
	}

	void Label::SetVariable(float* f)
	{
		float_to_watch = f;
	}

	void Label::SetVariable(double* d)
	{
		double_to_watch = d;
	}

	void Label::SetBaseMultiplier(int i)
	{
		base_mul_int = i;
	}

	void Label::SetBaseMultiplier(float f)
	{
		base_mul_float = f;
	}

	void Label::SetBaseMultiplier(double d)
	{
		base_mul_double = d;
	}

	void Label::SetPrecision(int decimal_points)
	{
		print_precision = decimal_points;
	}

	void Label::SetVariableMultiplier(int mul)
	{
		variable_multiplier = mul;
	}

	void Label::SetPrettyPrintFunc(pretty_print_func func)
	{
		print_func = func;
	}

	void Label::RaiseVariableValue()
	{
		if (int_to_watch)
		{
			*int_to_watch += variable_multiplier * base_mul_int;
		}
		else if (float_to_watch)
		{
			*float_to_watch += variable_multiplier * base_mul_float;
		}
		else if (double_to_watch)
		{
			*double_to_watch += variable_multiplier * base_mul_double;
		}
	}

	void Label::LowerVariableValue()
	{
		if (int_to_watch)
		{
			*int_to_watch -= variable_multiplier * base_mul_int;
		}
		else if (float_to_watch)
		{
			*float_to_watch -= variable_multiplier * base_mul_float;
		}
		else if (double_to_watch)
		{
			*double_to_watch -= variable_multiplier * base_mul_double;
		}
	}

}