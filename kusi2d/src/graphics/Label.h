#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <map>
#include <graphics/GLSLProgram.h>
#include <graphics/Vertex.h>
#include <graphics/SpriteBatch.h>
#include <graphics/Text.h>

#include <util/util.h>

typedef float (*pretty_print_func)(float);

namespace k2d
{
	class Label : public Text
	{
	public:
		/**
		 *	Constructor for Text, Input where you want the text to be (world coords) and spritebatch
		 */
		//Text(std::string _text, std::string _font_file, float _x, float _y, float _scale, float _depth, Color _color, SpriteBatch &_sprite_batch);
		Label(std::string _text, std::map<GLchar, Character>& _font, float _x, float _y,
			float _scale, float _depth, Color _color, SpriteBatch* _sprite_batch);
		~Label();

		/// Sends the text sprites to the spritebatch
		void Update();

		// Horrible code but whatever cba
		void SetVariable(int* i);
		void SetVariable(float* f);
		void SetVariable(double* d);

		void SetBaseMultiplier(int i);
		void SetBaseMultiplier(float f);
		void SetBaseMultiplier(double d);

		void SetTextOffset(k2d::vi2d text_offset);

		void SetPrecision(int decimal_points);

		void SetVariableMultiplier(int mul);

		void SetPrettyPrintFunc(pretty_print_func func);

		void RaiseVariableValue();
		void LowerVariableValue();

		/// Sets text to render
		void SetLabelText(std::string _text) { label_text = _text; }
		void SetPosition(vf2d pos) override { x = pos.x; y = pos.y; Text::SetPosition(pos + text_offset); }

		vf2d GetPosition() { return vf2d(x, y); }


	private:
		pretty_print_func print_func;
		
		std::string		label_text;
		int				variable_multiplier;

		int*			int_to_watch;
		float*			float_to_watch;
		double*			double_to_watch;

		int				print_precision;
		std::string		value;

		vi2d			text_offset;

		int base_mul_int;
		float base_mul_float;
		double base_mul_double;
	};


} // End of le namespace