#include <core/Window.h>

namespace k2d
{

	Window::Window() :
		window(nullptr),
		screen_width(NULL),
		screen_height(NULL),
		v_sync_enabled(false)
	{

	}

	Window::~Window()
	{

	}

	int Window::Create(std::string _window_name, int _screen_width, int _screen_height, unsigned int _window_flags, bool _v_sync)
	{
		screen_width = _screen_width;
		screen_height = _screen_height;

		Uint32 flags = SDL_WINDOW_OPENGL;

		if (_window_flags & INVISIBLE)
		{
			flags |= SDL_WINDOW_HIDDEN;
		}
		if (_window_flags & FULLSCREEN)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		if (_window_flags & BORDERLESS)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		// Create window
		window = SDL_CreateWindow(_window_name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screen_width, _screen_height, flags);
		if (window == nullptr)
		{
			KUSI_ERROR("SDL Window could not be created!");
		}

		// Setup GL context for window
		SDL_GLContext glContext = SDL_GL_CreateContext(window);
		if (glContext == nullptr)
		{
			KUSI_ERROR("SDL_GL context could not be created!");
		}

		// INIT GLEW
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			KUSI_ERROR("Could not initialize GLEW");
		}

		std::printf("***	OpenGL Version: %s   ***\n", glGetString(GL_VERSION));

		// Set background color
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

		SDL_GL_SetSwapInterval(1);

		// Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		_v_sync ? SDL_GL_SetSwapInterval(1) : SDL_GL_SetSwapInterval(0);

		return 0;
	}

	void Window::SetVSync(bool _v_sync)
	{
		_v_sync ? SDL_GL_SetSwapInterval(1) : SDL_GL_SetSwapInterval(0);
	}

	void Window::SwapBuffers()
	{
		SDL_GL_SwapWindow(window);
	}

	void Window::Destroy()
	{
		KUSI_DEBUG("Destroying window...");
		SDL_GL_DeleteContext(window);
	}

	void Window::SetTitle(std::string _title)
	{
		SDL_SetWindowTitle(window, _title.c_str());
	}

	SDL_Window* Window::GetSDLWindow()
	{
		return window;
	}

} // End of k2d namespace