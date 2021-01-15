#include <core/Engine.h>

namespace k2d
{
	// Forward declares for static functions
	static void sPerfClockInit(PerfClock* c, double target_delta);
	static double sPerfClockTick(PerfClock* c);
	static inline double sComputeDeltaMs(uint64_t tickTime, uint64_t lastTick,
		uint64_t performanceFrequency, double targetDelta);

	/// Creates and iniatilizes the engine
	Engine::Engine(std::string _window_title, int _window_width, int _window_height, bool _v_sync) :
		window(),
		running(false)
	{

		//graphics_system = GraphicsSystem();
		input_manager = InputManager();
		graphics_system = GraphicsSystem();
		window.Create(_window_title, _window_width, _window_height, 0, _v_sync);
		graphics_system.Init(Color(0,0,0, 255), &window);
		//graphics_system.Init(Color(255, 255, 0, 128), "Shaders/colorShading.vert", "Shaders/colorShading.frag");
	}

	Engine::~Engine()
	{

	}


	/// Starts the main loop
	int Engine::Run()
	{
		if (running)
		{
			return 1;
		}

		SetTargetFps(60);

		double tar_delta = 0.f;
		if (target_fps)
			tar_delta = 1.f / (double)target_fps;

		sPerfClockInit(&perf_clock, tar_delta);


		running = true;
		KUSI_DEBUG("Started running internal loop.\n");

		while (running)
		{
			this->Update();
		}

		return 0;
	}

	int Engine::RunExternalLoop(uint32_t _target_fps)
	{
		if (running)
		{
			return 1;
		}

		SetTargetFps(_target_fps);

		double tar_delta = 0.f;
		if (target_fps)
			tar_delta = 1.f / (double)target_fps;

		sPerfClockInit(&perf_clock, tar_delta);

		running = true;
		KUSI_DEBUG("Started running external loop.\n");

		return 0;
	}


	int Engine::StopRunning()
	{
		KUSI_DEBUG("Stop command received, stopping...");
		running = false;
		return 0;
	}

	void Engine::ReadyRendering()
	{
		graphics_system.ReadyRendering();
		ProcessInput();
	}

	/*
	 *	Main Update function
	 */
	double Engine::Update()
	{
		double delta_seconds = sPerfClockTick(&perf_clock);


		graphics_system.Tick(delta_seconds);
		


		// Swap buffers
		window.SwapBuffers();

		return delta_seconds;
	}

	/// Gets uptime in ticks?
	double Engine::GetUptime()
	{
		//Seconds
		// TODO: According to Microsoft's documentation, the high performance
		// clock may wrap around quickly. Use a millisecond tick timer like in SDL
		// instead? 
		return perf_clock.uptime;
	}

	void Engine::SetWindowTitle(std::string _title)
	{
		window.SetTitle(_title);
	}

	/// Sets the target FPS for the application
	void Engine::SetTargetFps(uint32_t fps)
	{
		target_fps = fps;
		perf_clock.target_delta = fps ? 1.f / (double)fps : 0.f;
	}

	/// Calculates average FPS over a specified time period 
	void Engine::CalculateFPS()
	{
		static const int NUM_SAMPLES = 60;
		static double frame_times[NUM_SAMPLES];
		static int current_frame = 0;

		frame_times[current_frame % NUM_SAMPLES] = perf_clock.delta;

		int count;

		current_frame++;
		if (current_frame < NUM_SAMPLES)
		{
			count = current_frame;
		}
		else
		{
			count = NUM_SAMPLES;
		}

		double frame_time_avg = 0.0f;

		for (int i = 0; i < count; i++)
		{
			frame_time_avg += frame_times[i];
		}

		frame_time_avg /= count;

		if (frame_time_avg > 0)
		{
			perf_clock.fps = 1.0 / frame_time_avg;
		}
		else
		{
			perf_clock.fps = 60.0;
		}
	}



	/*
	 *	Handles input and gives it to the InputManager
	 */
	void Engine::ProcessInput()
	{
		// Loops until no events to process
		while (SDL_PollEvent(&evnt))
		{
			switch (evnt.type)
			{
				// Also check for user pressing the window x button
			case SDL_QUIT:
				StopRunning();
				break;
			case SDL_MOUSEMOTION:
				input_manager.SetMouseCoords((float)evnt.motion.x, (float)evnt.motion.y);
				break;
			case SDL_KEYDOWN:
				input_manager.PressKey(evnt.key.keysym.sym);
				break;
			case SDL_KEYUP:
				input_manager.ReleaseKey(evnt.key.keysym.sym);
				break;
			case SDL_MOUSEBUTTONDOWN:
				input_manager.PressButton(evnt.button.button);
				break;
			case SDL_MOUSEBUTTONUP:
				input_manager.ReleaseButton(evnt.button.button);
				break;
			}
		}

		// Dont ship this: 
		// Pressing escape will stop engine
		if (input_manager.IsKeyPressed(SDLK_ESCAPE))
		{
			StopRunning();
		}
		if (input_manager.IsKeyPressed(SDLK_F1))
		{
			CalculateFPS();
			std::cout << perf_clock.fps << " fps\n";
		}
	}

	int Engine::AddShaders(const std::string& _vertex_shader_file, const std::string& _fragment_shader_file, const char* _id, std::initializer_list<std::string> _list)
	{
		graphics_system.AddShaders(_vertex_shader_file, _fragment_shader_file, _id, _list);
		return 0;
	}

	void Engine::MoveCamera(vf2d amount)
	{
		graphics_system.GetCamera()->setPosition(graphics_system.GetCamera()->getPosition() + amount);
	}

	// Gets a referance to the InputManager
	InputManager& Engine::GetInputManager()
	{
		return input_manager;
	}

	SpriteBatch* Engine::GetSpriteBatch()
	{
		return graphics_system.GetSpriteBatch();
	}

	/// Initializes performance clock
	static void sPerfClockInit(PerfClock* c, double targetDelta)
	{
		c->delta = 0;
		c->target_delta = targetDelta;
		c->last_tick = 0;
		c->counter_start = GetPerfCounter();
		c->performance_frequency = (uint64_t)GetPerfFrequency(); //ms
		c->uptime = 0;
	}

	/// Updates performance clock
	static double sPerfClockTick(PerfClock* c)
	{
		c->uptime += c->delta;
		uint64_t tick_time = GetPerfCounter();

		uint64_t performance_frequency = c->performance_frequency;
		double target_delta = c->target_delta * 1000.f;
		double delta = sComputeDeltaMs(tick_time, c->last_tick, performance_frequency,
			target_delta);
		if (delta < target_delta)
		{
			SleepMilliseconds((uint32_t)std::abs((target_delta - delta)));
			tick_time = GetPerfCounter();
			delta = sComputeDeltaMs(tick_time, c->last_tick, performance_frequency,
				target_delta);
		}
		c->last_tick = tick_time;
		c->delta = delta = delta / 1000.f;

		return delta;
	}

	// Calculates DeltaTime for each tick
	static inline double sComputeDeltaMs(uint64_t tickTime, uint64_t lastTick,
		uint64_t performanceFrequency, double targetDelta)
	{
		double delta;
		if (lastTick && lastTick < tickTime)
			delta = (double)(tickTime - lastTick) / (double)performanceFrequency;
		else
			delta = targetDelta;
		return delta * 1000.f;
	}

}
