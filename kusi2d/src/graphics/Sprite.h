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
			glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch);
		//Sprite(SpriteBatch& _sprite_batch);
		~Sprite();

		// Example call _-----------------------------------------------------
		//void Init() { Init(glm::vec3(0.f, 0.f, 0.f), 49.f, 49.f, 
		//glm::vec4(0.f, 0.f, 1.f, 1.f), Color(255, 0, 0, 255), ImageLoader::LoadPNG("Textures/loskatex.png")); }
		// ------------------------------------

		void Init(glm::vec2 _position, float _width, float _height, float depth,
			glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch);

		void SetUV(glm::vec4 _uv);
		void SetTexture(GLTexture _texture) { texture = _texture; }

		void Tick();
		void SetPosition(glm::vec2 _position) { position = _position; }
		void SetWidth(float width) { this->width = width; }
		void SetHeight(float height) { this->height = height; }
		void SetColor(Color new_color) { this->color = new_color; }

		void SetActive(bool a);


		glm::vec2 GetPosition() { return position; }
		glm::vec2 GetDimensions() { return glm::vec2(width, height); }

	private:
		GLTexture		texture;

		glm::vec2		position;
		float			width;
		float			height;
		float			depth;
		glm::vec4		uv_coordinates;
		SpriteBatch* 	sprite_batch;
		Color			color;

		bool			active;
		bool			initialized;
	};


}
