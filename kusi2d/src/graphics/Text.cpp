#include <graphics/Text.h>

namespace k2d
{

	//Text::Text(std::string _text, std::string _font_file, float _x, float _y, float _scale, float _depth, Color _color, SpriteBatch& _sprite_batch) :
	//	x(_x), y(_y), scale(_scale), color(_color), sprite_batch(&_sprite_batch), text(_text), font_family(_font_file), characters()
	//{
	//
	//}

	Text::Text(std::string _text, std::map<GLchar, Character>& _characters, float _x, float _y,
		float _scale, float _depth, Color _color, SpriteBatch* _sprite_batch) :
		x(_x), y(_y), scale(_scale), color(_color), sprite_batch(_sprite_batch), text(_text),
		characters(_characters), depth(_depth), fake_x(x), fake_y(y)
	{
	}

	Text::~Text()
	{

	}

	void Text::Update()
	{
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

}