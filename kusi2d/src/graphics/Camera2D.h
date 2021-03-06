#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <util/vecs.h>
#include <gl/glew.h>

namespace k2d
{
	//enum class direction { FORWARD = 0, BACKWARD, LEFT, RIGHT };

	class Camera2D
	{
	public:
		Camera2D();
		~Camera2D();

		void Init(int screenWidth, int screenHeight, vf2d pos);

		void Update();

		vf2d convertScreenToWorld(vf2d screenCoords);
		vf2d WorldToScreen(vf2d world);
		
		void Zoom(float amount) { _scale *= amount; _needsMatrixUpdate = true; }

		// Setters
		void setPosition(const vf2d& newPosition) { _position = newPosition; _needsMatrixUpdate = true; }
		void setScale(float newScale) { _scale = newScale; _needsMatrixUpdate = true; }

		// Getters
		vf2d getPosition() { return _position; }
		float getScale() { return _scale; }
		glm::mat4 getCameraMatrix() { return _cameraMatrix; }


	private:
		int _screenWidth, _screenHeight;
		bool _needsMatrixUpdate;
		float _scale;
		vf2d _position;
		glm::mat4 _cameraMatrix;
		glm::mat4 _orthoMatrix;
	};


}

