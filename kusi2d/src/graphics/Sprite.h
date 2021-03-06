//#include <core/ECS_Collection.h>
#pragma once
#include <string>
#include <graphics/GLTexture.h>
#include <graphics/SpriteBatch.h>
#include <glm/glm.hpp>
#include <graphics/GraphicsSystem.h>

namespace k2d
{
	/*
	 *	Spritecomponent for handling things we want to draw every frame
	 */
	class Sprite
	{
	public:
		Sprite();
		Sprite(glm::vec2 _position, float _width, float _height, float depth,
			glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch, float angle = 0.0f);
		Sprite(k2d::vf2d _position, k2d::vf2d size, float depth, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch, float angle = 0.0f);
		//Sprite(SpriteBatch& _sprite_batch);
		~Sprite();


		void Init(glm::vec2 _position, float _width, float _height, float depth,
			glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch);

		void SetUV(glm::vec4 _uv);
		void SetTexture(GLTexture _texture) { texture = _texture; }

		void Tick();
		void SetPosition(glm::vec2 _position) { position = _position; }
		void SetPosition(k2d::vf2d pos) { position.x = pos.x, position.y = pos.y; }
		void SetWidth(float width) { this->width = width; }
		void SetHeight(float height) { this->height = height; }
		void SetDepth(float depth) { this->depth = depth; }
		void SetColor(Color new_color) { this->color = new_color; }
		void SetAngle(float a) { this->angle = a; }
		void SetActive(bool a);


		glm::vec2 GetPosition() { return position; }
		glm::vec2 GetDimensions() { return glm::vec2(width, height); }

		k2d::vf2d GetPositionk2d() { return k2d::vf2d(position.x, position.y); }

	private:
		GLTexture		texture;

		glm::vec2		position;
		float			width;
		float			height;
		float			depth;
		glm::vec4		uv_coordinates;
		SpriteBatch* 	sprite_batch;
		Color			color;

		float			angle;
		bool			active;
		bool			initialized;
	};


}
