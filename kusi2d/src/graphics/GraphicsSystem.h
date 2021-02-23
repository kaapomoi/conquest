#pragma once
#include <graphics/SpriteBatch.h>
#include <graphics/GLSLProgram.h>
#include <util/ImageLoader.h>
#include <util/util.h>
#include <core/Window.h>

// Quick test
#include <graphics/Label.h>
#include <graphics/Sprite.h>
#include <graphics/Camera2D.h>
#include <vector>
#include <map>

namespace k2d
{
	/**
	 *	Graphics system.
	 *	Updates & draws all graphics components
	 */
	class GraphicsSystem
	{
	public:
		GraphicsSystem();
		GraphicsSystem(const GraphicsSystem&) = default;
		GraphicsSystem& operator=(const GraphicsSystem&) = default;
		~GraphicsSystem();

		void Init(Color _backgound_color, Window* _window);

		void ReadyRendering();

		void Tick(double _delta_seconds);

		void SetBackgroundColor(k2d::Color _color);

		SpriteBatch* GetSpriteBatch();

		void AddShaders(const std::string& _vertex_shader_file,
			const std::string& _fragment_shader_file,
			const char* _id,
			std::initializer_list<std::string> _list);

		Camera2D* GetCamera() { return &camera; }

	private:
		Window*			window;

		Color			background_color;
		SpriteBatch*	sprite_batch;
		std::map<const char*, GLSLProgram*> glsl_programs;
		GLSLProgram*	current_prog;
		bool			shader_compile_success;
		bool			initialized;

		// Quick test
		Camera2D						camera;
	};

}