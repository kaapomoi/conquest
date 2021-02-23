#pragma once
#include <unordered_map>
#include <util/vecs.h>
#include <SDL\SDL_stdinc.h>

namespace k2d
{

	enum class WheelDirection
	{
		UP,
		DOWN
	};

	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void Update();

		void PressKey(int _key_id);

		void PressButton(Uint8 id);
		void ReleaseButton(Uint8 id);

		void ReleaseKey(int _key_id);

		void SetMouseCoords(float _x, float _y);

		void ScrollWheel(WheelDirection d);

		bool IsKeyPressed(int _key_id);
		bool IsButtonPressed(Uint8 id);

		bool IsKeyPressedThisFrame(int key_id);
		bool IsButtonPressedThisFrame(Uint8 id);
		bool IsMouseWheelScrolledThisFrame(WheelDirection d);

		// Getters
		vf2d GetMouseCoords() const { return mouse_coords; }

	private:
		std::pair<bool, bool> mouse_scroll_up_down_this_frame;
		std::unordered_map<int, bool> key_map;
		std::unordered_map<Uint8, bool> button_map;
		vf2d mouse_coords;

		std::unordered_map<int, bool> key_map_this_frame;
		std::unordered_map<Uint8, bool> button_map_this_frame;
	};

} // End of namespace k2d

