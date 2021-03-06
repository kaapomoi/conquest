#pragma once

#include <util/util.h>
#include <core/Engine.h>

namespace k2d
{

	class Application
	{
	public:
		Application(std::string title, int width, int height, int target_fps, bool v_sync);
		virtual	~Application();

		virtual void Setup();

		virtual void SetShaders(std::string vert_file, std::string frag_file, std::string id, std::initializer_list<std::string> attributes);
		
		virtual void PreRender();

		virtual void Update();


	protected:
		Engine* engine;
		SpriteBatch* sprite_batch;

		float fps_target;
		double dt;

		int window_width;
		int window_height;

		// Fonts
		std::map<const char*, std::map<GLchar, k2d::Character>>	font_cache;
		std::map<GLchar, k2d::Character> font1;
		FT_Library		ft;
		FT_Face			face;

		std::map<GLchar, k2d::Character> LoadFont(const char* _file);
		std::map<GLchar, k2d::Character> LoadChars();

		// Texture cache
		std::map<const char*, k2d::GLTexture> m_texture_cache;

		int load_texture_into_cache(const char* friendly_name, std::string filename);
		k2d::GLTexture load_texture_from_cache(const char* friendly_name);

	private:

		void MainLoop();

	};

} // End of k2d namespace