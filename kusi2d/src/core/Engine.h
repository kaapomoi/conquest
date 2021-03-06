#pragma once
#include <core/Win32_System.h>

#include <util/InputManager.h>
#include <graphics/GraphicsSystem.h>
#include <core/Window.h>

namespace k2d
{
	/// Performance clock struct containing time values
	struct PerfClock
	{
		double delta;
		double target_delta;
		double uptime;
		double fps;
		uint64_t last_tick;
		uint64_t performance_frequency;
		uint64_t counter_start;
	};

	/**
	 * Base Engine class, creates window,
	 * handles main loop, sends input to the input_manager
	 */
	class Engine
	{
	public:
		// Test
		Engine(std::string _window_title, int _window_width, int _window_height, bool _v_sync);
		~Engine();

		int Run();
		int RunExternalLoop(uint32_t _target_fps);
		int StopRunning();

		void PreRender(const char* id);

		double Update();

		void ProcessInput();

		int AddShaders(const std::string& _vertex_shader_file,
			const std::string& _fragment_shader_file,
			const char* _id,
			std::initializer_list<std::string> _list);

		void MoveCamera(vf2d amount);
		void SetCameraPosition(vi2d new_pos) { graphics_system.GetCamera()->setPosition(new_pos); }
		void ZoomCamera(float amount) { graphics_system.GetCamera()->Zoom(amount); }

		InputManager& GetInputManager();
		SpriteBatch* GetSpriteBatch();

		double GetUptime(); //TODO: change to a millisecond timer (unsigned int)
		double GetDeltaTime() { return perf_clock.delta; }

		void SetWindowTitle(std::string _title);

		///Set the target frame rate. A value of 0 indicates framerate is unlocked.
		void SetTargetFps(uint32_t fps);
		uint32_t GetTargetFps() { return target_fps; }

		void CalculateFPS();

		double GetCurrentFPS() { return perf_clock.fps; }
		bool GetRunning() { return running; }

		vf2d GetMouseCoords() { return input_manager.GetMouseCoords(); }
		vf2d ScreenToWorld(vf2d screen) { return graphics_system.GetCamera()->convertScreenToWorld(screen); }
		vf2d WorldToScreen(vf2d world) { return graphics_system.GetCamera()->WorldToScreen(world); }

	private:
		bool			running;
		InputManager	input_manager;
		GraphicsSystem  graphics_system;
		Window			window;
		SDL_Event		evnt;

		PerfClock		perf_clock;
		uint32_t		target_fps;


	};

}