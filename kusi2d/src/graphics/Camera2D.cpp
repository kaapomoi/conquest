#include <graphics/Camera2D.h>

#include <iostream>

namespace k2d
{
	Camera2D::Camera2D() :
		_position(0.0f, 0.0f),
		_cameraMatrix(1.0f),
		_orthoMatrix(1.0f),
		_scale(1.0f),
		_needsMatrixUpdate(true),
		_screenWidth(0),
		_screenHeight(0)
	{

	}

	Camera2D::~Camera2D()
	{

	}

	void Camera2D::Init(int screenWidth, int screenHeight, vf2d pos)
	{
		_screenWidth = screenWidth;
		_screenHeight = screenHeight;
		_position = pos;
		_orthoMatrix = glm::ortho(0.0f, (float)_screenWidth, 0.0f, (float)_screenHeight);
	}

	void Camera2D::Update()
	{
		if (_needsMatrixUpdate)
		{
			glm::vec3 translate(-_position.x + _screenWidth / 2.0f, -_position.y + _screenHeight / 2.0f, 0.0f);
			_cameraMatrix = glm::translate(_orthoMatrix, translate);

			glm::vec3 scale(_scale, _scale, 0.0f);
			_cameraMatrix = glm::scale(_cameraMatrix, scale);

			_needsMatrixUpdate = false;
		}
	}

	vf2d Camera2D::convertScreenToWorld(vf2d screenCoords)
	{
		screenCoords.y = _screenHeight - screenCoords.y;
		//0 is center
		screenCoords -= vf2d(_screenWidth / 2.0f, _screenHeight / 2.0f);
		// scale coords
		screenCoords /= _scale;
		// Translate with the camera pos
		screenCoords += _position;
		return screenCoords;
	}

	vf2d Camera2D::WorldToScreen(vf2d world)
	{
		world.y = _screenHeight - world.y;
		world -= vf2d(_screenWidth / 2.0f, _screenHeight / 2.0f);
		world /= _scale;
		world += _position;

		return world;
	}
}