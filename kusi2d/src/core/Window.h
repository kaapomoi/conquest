#pragma once
#include <GL/glew.h>
#include <string>
#include <SDL/SDL.h>
#include <util/util.h>

namespace k2d
{

	enum WINDOW_FLAGS
	{
		INVISIBLE = 0x1,
		FULLSCREEN = 0x2,
		BORDERLESS = 0x4,
	};

	class Window
	{
	public:
		Window();
		~Window();

		int Create(std::string _window_name, int _screen_width, int _screen_height, unsigned int _window_flags, bool _v_sync);

		void SetVSync(bool _v_sync);

		void SwapBuffers();

		void Destroy();

		void SetTitle(std::string _title);

		SDL_Window* GetSDLWindow();

		vi2d GetDimensions() const { return vi2d(screen_width, screen_height); }

	private:
		SDL_Window* window;
		int			screen_width;
		int			screen_height;
		bool		v_sync_enabled;
	};

} // End of k2d namespace