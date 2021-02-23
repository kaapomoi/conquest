#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <map>
#include <graphics/GLSLProgram.h>
#include <graphics/Vertex.h>
#include <graphics/SpriteBatch.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <util/util.h>

namespace k2d
{
	struct Character
	{
		GLuint		texture_id;
		glm::ivec2	size;
		glm::ivec2	bearing;
		GLuint		advance;
	};

	class Text
	{
	public:
		/**
		 *	Constructor for Text, Input where you want the text to be (world coords) and spritebatch
		 */
		//Text(std::string _text, std::string _font_file, float _x, float _y, float _scale, float _depth, Color _color, SpriteBatch &_sprite_batch);
		Text(std::string _text, std::map<GLchar, Character>& _font, float _x, float _y,
			float _scale, float _depth, Color _color, SpriteBatch* _sprite_batch);
		~Text();

		/// Sends the text sprites to the spritebatch
		void Update();

		/// Sets text to render
		void SetText(std::string _text) { text = _text; }
		void SetPosition(vf2d pos) { x = pos.x; y = pos.y; }

		std::string GetText() { return text; }

		float GetDepth() { return depth; }

		vf2d GetPosition() { return vf2d(x, y); }
		vf2d GetDimensions() { return vf2d(text.length() * 50.0f, 90.0f); }

	protected:
		std::map<GLchar, Character>& characters;

		std::string		text;
		SpriteBatch*	sprite_batch;
		
		float			x;
		float			y;
		float			fake_x;
		float			fake_y;

		float			scale;
		float			depth;
		Color			color;
	};




} // End of le namespace