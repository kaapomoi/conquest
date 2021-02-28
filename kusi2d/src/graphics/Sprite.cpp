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
		active(true)
	{
		//std::cout << "addr" << &sprite_batch << "\n";
	}

	Sprite::Sprite(glm::vec2 _position, float _width, float _height, float depth,
		glm::vec4 _uv_coordinates, Color _color, GLTexture _texture, SpriteBatch* _sprite_batch)
	{
		position = _position;
		width = _width;
		height = _height;
		uv_coordinates = _uv_coordinates;
		color = _color;
		texture = _texture;
		sprite_batch = _sprite_batch;
		initialized = true;
		this->depth = depth;
		this->active = true;
	}

	/// Default constructor
	//Sprite::Sprite(SpriteBatch& _sprite_batch) :
	//	offset(0.0f),
	//	width(0.0f),
	//	height(0.0f),
	//	uv_coordinates(0.0f),
	//	sprite_batch(_sprite_batch),
	//	texture()
	//	//sprite_batch(static_cast<GraphicsSystem*>(this->GetSystem())->GetSpriteBatch())
	//{

	//}

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
			// Create destination rectangle for sprite
			glm::vec4 dest_rect((-width / 2.0f) + position.x, (-height / 2.0f) + position.y, width, height);
			//glm::vec4 dest_rect(position.x, position.y, width, height);
		

			// Add the sprite to the spritebatch
			sprite_batch->Draw(dest_rect, uv_coordinates, texture.id, color, depth);
		}
	}

	void Sprite::SetActive(bool a)
	{
		active = a;
	}

}
