#pragma once

#include <GL/glew.h>
namespace k2d
{
	struct Position
	{
		float x;
		float y;
	};

	struct Color
	{
		/**
		 *	0-255 value for each red, green, blue & alpha channels
		 */
		GLubyte r;
		GLubyte g;
		GLubyte b;
		GLubyte a;
		
		/// If no color is set, default to magenta.
		Color()
		{
			r = 255;
			g = 0;
			b = 255;
			a = 255;
		};
		Color(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a)
		{
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		};
		Color(GLubyte v)
		{
			r = v;
			g = v;
			b = v;
			a = 255;
		}
		Color(GLubyte v, GLubyte a)
		{
			r = v; 
			g = v;
			b = v;
			this->a = a;
		}
	};

	struct UV
	{
		float u;
		float v;
	};

	struct Vertex
	{
		Position position;

		Color color;

		UV uv;

		void setPosition(float x, float y) {
			position.x = x;
			position.y = y;
		}
		
		void SetPosition(glm::vec2 pos) {
			position.x = pos.x;
			position.y = pos.y;
		}

		Position GetPosition() { return position; }

		void setColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
			color.r = r;
			color.g = g;
			color.b = b;
			color.a = a;
		}

		void setUV(float u, float v) {
			uv.u = u;
			uv.v = v;
		}
	};
}