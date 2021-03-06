#include <core/Application.h>

namespace k2d
{

	Application::Application(std::string title, int width, int height, int target_fps, bool v_sync)
	{
		engine = new Engine(title, width, height, v_sync);
		sprite_batch = engine->GetSpriteBatch();
		engine->SetTargetFps(target_fps);
		window_width = width;
		window_height = height;
		fps_target = target_fps;

		face = 0;
		ft = 0;

		dt = 0.0;

	}

	Application::~Application()
	{
		delete engine;
		delete sprite_batch;
	}

	void Application::Setup()
	{


		engine->RunExternalLoop(fps_target);
		MainLoop();
	}

	void Application::SetShaders(std::string vert_file, std::string frag_file, std::string id, std::initializer_list<std::string> attributes)
	{
		engine->AddShaders(vert_file, frag_file, id.c_str(), attributes);
	}

	void Application::PreRender()
	{
		engine->PreRender("core");
	}

	void Application::Update()
	{
		dt = engine->Update();
	}

	void Application::MainLoop()
	{
		while (engine->GetRunning())
		{
			PreRender();


			Update();
		}
	}

	/**
	 *	Loads a font from the texture cache.\n
	 *	If the font is not in the cache, \n
	 *	it gets loaded into it from the specified file
	 */
	std::map<GLchar, k2d::Character> Application::LoadFont(const char* _file)
	{
		if (FT_Init_FreeType(&ft))
		{
			k2d::KUSI_ERROR("ERROR::FREETYPE: Could not init Freetype Library");
		}

		if (FT_New_Face(ft, _file, 0, &face))
		{
			k2d::KUSI_ERROR("ERROR::FREETYPE: Failed to load font");
		}

		FT_Set_Pixel_Sizes(face, 0, 128);

		// Lookup texturemap
		auto mit = font_cache.find(_file);


		// If its not in the map
		if (mit == font_cache.end())
		{
			std::map<GLchar, k2d::Character> new_map = LoadChars();

			// Insert it into the map
			font_cache.insert(std::make_pair(_file, new_map));
			return new_map;
		}

		// return texture
		return mit->second;
	}

	/**
	 *	Loads characters of a font into a usable format
	 */
	std::map<GLchar, k2d::Character> Application::LoadChars()
	{
		std::map<GLchar, k2d::Character> characters = {};

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		
		
		// Try for the first 128 chars
		for (GLubyte c = 0; c < 128; c++)
		{
			// Try to load glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				k2d::KUSI_DEBUG("ERROR::FREETYPE: Failed to load Glyph");
				continue;
			}

			

			// Generate texture
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA8,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_ALPHA,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Now store character for later use
			k2d::Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			characters.insert(std::pair<GLchar, k2d::Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Free memory
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return characters;
	}

	int Application::load_texture_into_cache(const char* friendly_name, std::string filename)
	{
		// Lookup texturemap
		auto mit = m_texture_cache.find(friendly_name);

		// If its not in the map
		if (mit == m_texture_cache.end())
		{
			k2d::GLTexture tex = k2d::ImageLoader::LoadPNG(filename, false);

			// Insert it into the map
			m_texture_cache.insert(std::make_pair(friendly_name, tex));
		}

		return 0;
	}

	k2d::GLTexture Application::load_texture_from_cache(const char* friendly_name)
	{
		// Lookup texturemap
		auto mit = m_texture_cache.find(friendly_name);

		// If its not in the map
		if (mit == m_texture_cache.end())
		{
			k2d::KUSI_DEBUG("Cannot find texture from cache, name: %s\nCreating new default texture", friendly_name);
			// Make new texture from default image
			return k2d::ImageLoader::LoadPNG("Textures/default.png", false);
		}

		return mit->second;
	}


} // End of k2d namespace