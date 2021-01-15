#include <graphics/GraphicsSystem.h>

namespace k2d
{
	/**
	 *	Default Constructor for GraphicsSystem
	 */
	GraphicsSystem::GraphicsSystem() :
		initialized(false),
		background_color(),
		shader_compile_success(false),
		window(),
		current_prog()
	{
		sprite_batch = new SpriteBatch();
	}

	GraphicsSystem::~GraphicsSystem()
	{

	}

	/**
	 *	Initializes the graphics system, loads & compiles given shaders
	 */
	void GraphicsSystem::Init(Color _background_color, Window* _window)
	{
		window = _window;
		//std::cout << "gs addr init: " << &sprite_batch << "\n";
		if (initialized)
		{
			return;
		}
		background_color = _background_color;

		//glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		sprite_batch->Init();


		KUSI_DEBUG("Initialized: %d\n", (int)initialized);

		float width = (float)window->GetDimensions().x;
		float height = (float)window->GetDimensions().y;

		camera.Init(width, height, vf2d(width / 4, height / 3));

		// If all goes well, 
		initialized = true;
	}


	// Starts the batch and updates the camera
	void GraphicsSystem::ReadyRendering()
	{
		// Errorchecking, If we haven't initialized, crash engine
		if (!initialized)
		{
			KUSI_ERROR("Graphics System needs to be initialized first!");
		}

		glClearDepth(1.0);

		// Clear buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Clear screen to Background color
		//std::printf("bg: %f, %f, %f, %f", background_color.r / 255.f, background_color.g / 255.f,
			//background_color.b / 255.f, background_color.a / 255.f);
		glClearColor(background_color.r / 255.f, background_color.g / 255.f,
			background_color.b / 255.f, background_color.a / 255.f);


		const char* id = "2d";
		// Start rendering, change shaders to 3d

		if (glsl_programs.find(id) != glsl_programs.end())
		{
			current_prog = glsl_programs.find(id)->second;

			current_prog->Use();
		}
		else
		{
			KUSI_ERROR("No shader found with id");
		}

		// Camera test
		camera.Update();

		// Get camera uniform location and place the camera matrix into it
		//GLint view_matrix_location = current_prog->GetUniformLocation("camera");
		current_prog->SetMat4fv(camera.getCameraMatrix(), "camera", GL_FALSE);

		// Setup textures
		glActiveTexture(GL_TEXTURE0);
		//GLint texture_location = current_prog->GetUniformLocation("k2d_texture");
		//glUniform1i(texture_location, 0);
		current_prog->Set1i(GL_TEXTURE0, "k2d_texture");

		current_prog->Use();

		// Start batching
		sprite_batch->Begin(GlyphSortType::FRONT_TO_BACK);
	}

	/**
	 *	Main Rendering function, draws spritebatches
	 */
	void GraphicsSystem::Tick(double _delta_seconds)
	{
		// End spritebatch
		sprite_batch->End();
		
		// Draw triangles with spritebatch
		sprite_batch->RenderBatches();

		glActiveTexture(0);

		// Unbind GLSL program
		current_prog->UnUse();
	}


	

	/// Sets the windows background color
	void GraphicsSystem::SetBackgroundColor(k2d::Color _color)
	{
		background_color = _color;
	}

	/// Gets a reference to the spritebatch
	SpriteBatch* GraphicsSystem::GetSpriteBatch()
	{
		//std::cout << "what is this addr getspritebatch " << &sprite_batch << "\n";
		return sprite_batch;
	}

	/*
	 *	Shader Loading and compiling, sets the program to this graphicssystem
	 */
	void GraphicsSystem::AddShaders(const std::string& _vertex_shader_file,
		const std::string& _fragment_shader_file,
		const char* _id,
		std::initializer_list<std::string> _list)
	{
		auto mit = glsl_programs.find(_id);

		// If its not in the map
		if (mit == glsl_programs.end())
		{
			KUSI_DEBUG("Creating shaders with id %s\n", _id);
			GLSLProgram* prog = new GLSLProgram();
			shader_compile_success = prog->CompileShaders(_vertex_shader_file, _fragment_shader_file);

			if (!shader_compile_success)
			{
				KUSI_ERROR("Could not compile shaders\n");
			}

			for (auto elem : _list)
			{
				prog->AddAttribute(elem);
			}

			//glsl_program.AddAttribute("mvp");
			shader_compile_success = prog->LinkShaders();
			if (!shader_compile_success)
			{
				KUSI_ERROR("Could not link shaders");
			}

			// Insert it into the map
			glsl_programs.insert(std::make_pair(_id, prog));
		}

	}

}