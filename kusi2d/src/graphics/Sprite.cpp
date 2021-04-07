#include <graphics/Sprite.h>

namespace k2d
{
	Sprite::Sprite() :
		position(0.0f),
		width(0.0f),
		height(0.0f),
		uv_coordinates(0.0f),
		sprite_batch(0),
		texture(),
		initialized(false),
		depth(0),
		active(true),
		angle(0.0f)
	{

	}

	Sprite::Sprite(glm::vec2 _position, float _width, float _height, float depth,
		glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch, float angle)
	{
		position = _position;
		width = _width;
		height = _height;
		uv_coordinates = _uv_coordinates;
		color = _color;
		texture = _texture;
		sprite_batch = _sprite_batch;
		initialized = true;
		this->angle = angle;
		this->depth = depth;
		this->active = true;
	}

	Sprite::Sprite(k2d::vf2d _position, k2d::vf2d size, float depth, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch, float angle)
	{
		position = glm::vec2(_position.x, _position.y);
		width = size.x;
		height = size.y;
		uv_coordinates = glm::vec4(0, 0, 1, 1);
		color = _color;
		texture = _texture;
		sprite_batch = _sprite_batch;
		initialized = true;
		this->angle = angle;
		this->depth = depth;
		this->active = true;
	}

	/// Destructor
	Sprite::~Sprite()
	{

	}

	/// Initializes the component with given values
	void Sprite::Init(glm::vec2 _position, float _width, float _height, float depth,
		glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch)
	{
		position = _position;
		width = _width;
		height = _height;
		uv_coordinates = _uv_coordinates;
		color = _color;
		texture = _texture;
		sprite_batch = _sprite_batch;
		this->depth = depth;
		initialized = true;
		this->active = true;
	}

	/// Sets UV, used by AnimationComponent
	void Sprite::SetUV(glm::vec4 _uv)
	{
		uv_coordinates = _uv;
	}

	/// Update funtion adds the sprite to the spritebatch
	void Sprite::Tick()
	{
		if (!initialized)
		{
			KUSI_ERROR("SPRITE not initialized, error...");
		}
		
		if (active)
		{

			if (angle < 0.00001f && angle > -0.00001f)
			{
				glm::vec4 dest_rect((-width / 2.0f) + position.x, (-height / 2.0f) + position.y, width, height);

				sprite_batch->Draw(dest_rect, uv_coordinates, texture.id, color, depth);
			}
			else
			{
				// Create destination rectangle for sprite
				glm::vec4 dest_rect((-width / 2.0f) + position.x, (-height / 2.0f) + position.y, width, height);


				// Add the sprite to the spritebatch
				sprite_batch->DrawAngled(dest_rect, uv_coordinates, texture.id, color, depth, -angle);
			}
		
		}
	}

	void Sprite::SetActive(bool a)
	{
		active = a;
	}

}
