#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>

#include <util/util.h>
#include <graphics/Vertex.h>
#include <graphics/GLTexture.h>

namespace k2d
{

	enum class GlyphSortType {
		NONE,
		FRONT_TO_BACK,
		BACK_TO_FRONT,
		TEXTURE
	};

	struct Glyph
	{
		// pos uv
		Vertex topLeft;
		Vertex bottomLeft;
		Vertex topRight;
		Vertex bottomRight;

		GLuint texture;
		float depth;
	};

	class RenderBatch
	{
	public:
		RenderBatch(GLuint _offset, GLuint _num_vertices, GLuint _texture)
			: offset(_offset), num_vertices(_num_vertices), texture(_texture)
		{ }
		GLuint offset;
		GLuint num_vertices;
		GLuint texture;

	};

	class SpriteBatch
	{
	public:
		/// Default Constructor for SpriteBatch
		SpriteBatch();
		~SpriteBatch();

		/**
		 *  Call this first!
		 *  Creates vao, vbo
		 */
		void Init();

		/// Sets sort type, clears previous batches & glyphs
		void Begin(GlyphSortType _sort_type = GlyphSortType::TEXTURE);

		/// Sorts glyphs and creates creates batches
		void End();

		/// Makes a drawable object into a glyph, which will get put into a renderbatch in CreateRenderBatches()
		void Draw(const glm::vec4& _dest_rect, const glm::vec4& _uv_rect, 
			GLuint _texture, const Color& _color, float _depth);

		/// Draws all triangles of all spritebatches
		void RenderBatches();

	private:
		/// Makes renderable batches of glyphs
		void CreateRenderBatches();

		/// Creates Vertex array and buffer objects
		void CreateVertexArray();

		/// Sorts glyphs based on declared sort type
		void SortGlyphs();

		GLuint vbo;
		GLuint vao;

		GlyphSortType sort_type;

		std::vector<Glyph*> glyphs;
		std::vector<RenderBatch> render_batches;
	};


}