#include <util/InputManager.h>

namespace k2d 
{

	InputManager::InputManager() : mouse_coords(0.0f)
	{
		
	}

	InputManager::~InputManager()
	{

	}

	void InputManager::Update()
	{
		key_map_this_frame.clear();
		button_map_this_frame.clear();

		mouse_scroll_up_down_this_frame.first = false;
		mouse_scroll_up_down_this_frame.second = false;
	}

	void InputManager::PressKey(int _key_id)
	{
		key_map[_key_id] = true;
		key_map_this_frame[_key_id] = true;
	}

	void InputManager::ReleaseKey(int _key_id)
	{
		key_map[_key_id] = false;
	}

	void InputManager::PressButton(Uint8 id)
	{
		button_map[id] = true;
		button_map_this_frame[id] = true;
	}

	void InputManager::ReleaseButton(Uint8 id)
	{
		button_map[id] = false;
	}


	void InputManager::SetMouseCoords(float _x, float _y)
	{
		mouse_coords.x = _x;
		mouse_coords.y = _y;
	}

	void InputManager::ScrollWheel(WheelDirection d)
	{
		if (d == WheelDirection::UP)
		{
			mouse_scroll_up_down_this_frame.first = true;
		}
		else if (d == WheelDirection::DOWN)
		{
			mouse_scroll_up_down_this_frame.second = true;
		}
	}

	bool InputManager::IsKeyPressed(int _key_id)
	{
		auto it = key_map.find(_key_id);

		if (it != key_map.end())
		{
			return it->second;
		}
		else
		{
			return false;
		}
	}

	bool InputManager::IsButtonPressed(Uint8 id)
	{
		auto it = button_map.find(id);

		if (it != button_map.end())
		{
			return it->second;
		}
		else
		{
			return false;
		}
	}

	bool InputManager::IsKeyPressedThisFrame(int key_id)
	{
		auto it = key_map_this_frame.find(key_id);

		if (it != key_map_this_frame.end())
		{
			return it->second;
		}
		else
		{
			return false;
		}
	}

	bool InputManager::IsButtonPressedThisFrame(Uint8 b_id)
	{
		auto it = button_map_this_frame.find(b_id);

		if (it != button_map_this_frame.end())
		{
			return it->second;
		}
		else
		{
			return false;
		}
	}

	bool InputManager::IsMouseWheelScrolledThisFrame(WheelDirection d)
	{
		if (d == WheelDirection::UP)
		{
			return mouse_scroll_up_down_this_frame.first;
		}
		else if (d == WheelDirection::DOWN)
		{
			return mouse_scroll_up_down_this_frame.second;
		}
	}

} // End of namespace k2d