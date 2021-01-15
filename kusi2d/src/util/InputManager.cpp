#include <util/InputManager.h>

namespace k2d 
{

	InputManager::InputManager() : mouse_coords(0.0f)
	{
		
	}

	InputManager::~InputManager()
	{

	}

	void InputManager::PressKey(int _key_id)
	{
		key_map[_key_id] = true;
	}

	void InputManager::PressButton(Uint8 id)
	{
		button_map[id] = true;
	}

	void InputManager::ReleaseButton(Uint8 id)
	{
		button_map[id] = false;
	}

	void InputManager::ReleaseKey(int _key_id)
	{
		key_map[_key_id] = false;
	}

	void InputManager::SetMouseCoords(float _x, float _y)
	{
		mouse_coords.x = _x;
		mouse_coords.y = _y;
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

} // End of namespace k2d