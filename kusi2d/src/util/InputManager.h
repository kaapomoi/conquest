#pragma once
#include <unordered_map>
#include <util/vecs.h>
#include <SDL\SDL_stdinc.h>

namespace k2d
{

	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void PressKey(int _key_id);

		void PressButton(Uint8 id);
		void ReleaseButton(Uint8 id);

		void ReleaseKey(int _key_id);

		void SetMouseCoords(float _x, float _y);

		bool IsKeyPressed(int _key_id);
		bool IsButtonPressed(Uint8 id);

		// Getters
		vf2d GetMouseCoords() const { return mouse_coords; }

	private:
		std::unordered_map<int, bool> key_map;
		std::unordered_map<Uint8, bool> button_map;
		vf2d mouse_coords;
	};

} // End of namespace k2d

