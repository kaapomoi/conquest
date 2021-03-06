#include <graphics/SpriteBatch.h>

namespace k2d
{
	/**
	 *	Default Constructor for SpriteBatch
	 */
	SpriteBatch::SpriteBatch()
		: vao(0), vbo(0), sort_type(GlyphSortType::FRONT_TO_BACK)
	{
		
	}

	SpriteBatch::~SpriteBatch()
	{
	}

	/**
	 *	Call this first!
	 *  Creates vao, vbo
	 */
	void SpriteBatch::Init()
	{
		CreateVertexArray();
	}

	/** 
	 *  Sets sort type, clears previous batches & glyphs
	 */
	void SpriteBatch::Begin(GlyphSortType _sort_type)
	{
		sort_type = _sort_type;
		render_batches.clear();
		for (int i = 0; i < (int)glyphs.size(); i++)
		{
			delete glyphs[i];
		}
		glyphs.clear();
	}

	/**
	 *	Sorts glyphs and creates creates batches
	 */
	void SpriteBatch::End()
	{
		SortGlyphs();
		CreateRenderBatches();
	}

	/**
	 *	Makes a drawable object into a glyph, which will get put into a renderbatch in CreateRenderBatches()
	 */
	void SpriteBatch::Draw(const glm::vec4& _dest_rect, const glm::vec4& _uv_rect, GLuint _texture, const Color& _color, float _depth)
	{
		Glyph* newGlyph = new Glyph;
		newGlyph->texture = _texture;
		newGlyph->depth = _depth;

		newGlyph->topLeft.color = _color;
		newGlyph->topLeft.setPosition(_dest_rect.x, _dest_rect.y + _dest_rect.w);
		newGlyph->topLeft.setUV(_uv_rect.x, _uv_rect.y + _uv_rect.w);

		newGlyph->bottomLeft.color = _color;
		newGlyph->bottomLeft.setPosition(_dest_rect.x, _dest_rect.y);
		newGlyph->bottomLeft.setUV(_uv_rect.x, _uv_rect.y);

		newGlyph->bottomRight.color = _color;
		newGlyph->bottomRight.setPosition(_dest_rect.x + _dest_rect.z, _dest_rect.y);
		newGlyph->bottomRight.setUV(_uv_rect.x + _uv_rect.z, _uv_rect.y);

		newGlyph->topRight.color = _color;
		newGlyph->topRight.setPosition(_dest_rect.x + _dest_rect.z, _dest_rect.y + _dest_rect.w);
		newGlyph->topRight.setUV(_uv_rect.x + _uv_rect.z, _uv_rect.y + _uv_rect.w);

		glyphs.push_back(newGlyph);
	}

	void SpriteBatch::DrawAngled(const glm::vec4& _dest_rect, const glm::vec4& _uv_rect, GLuint _texture, const Color& _color, float _depth, float _angle)
	{
		float rad = k2d::DegToRad(_angle);
		glm::vec2 c(_dest_rect.x, _dest_rect.y);
		glm::mat2 rot_mat(glm::vec2(cos(rad), -sin(rad)), glm::vec2(sin(rad), cos(rad)));

		Glyph* newGlyph = new Glyph;
		newGlyph->texture = _texture;
		newGlyph->depth = _depth;

		newGlyph->topLeft.color = _color;
		glm::vec2 top_left_tmp(_dest_rect.x, _dest_rect.y + _dest_rect.w);
		top_left_tmp = rot_mat * (top_left_tmp - c) + c;
		newGlyph->topLeft.SetPosition(top_left_tmp);
		newGlyph->topLeft.setUV(_uv_rect.x, _uv_rect.y + _uv_rect.w);

		newGlyph->bottomLeft.color = _color;
		glm::vec2 bot_left_tmp(_dest_rect.x, _dest_rect.y);
		bot_left_tmp = rot_mat * (bot_left_tmp - c) + c;
		newGlyph->bottomLeft.SetPosition(bot_left_tmp);
		newGlyph->bottomLeft.setUV(_uv_rect.x, _uv_rect.y);

		newGlyph->bottomRight.color = _color;
		glm::vec2 bot_right_tmp(_dest_rect.x + _dest_rect.z, _dest_rect.y);
		bot_right_tmp = rot_mat * (bot_right_tmp - c) + c;
		newGlyph->bottomRight.SetPosition(bot_right_tmp);
		newGlyph->bottomRight.setUV(_uv_rect.x + _uv_rect.z, _uv_rect.y);

		newGlyph->topRight.color = _color;
		glm::vec2 top_right_tmp(_dest_rect.x + _dest_rect.z, _dest_rect.y + _dest_rect.w);
		top_right_tmp = rot_mat * (top_right_tmp - c) + c;
		newGlyph->topRight.SetPosition(top_right_tmp);
		newGlyph->topRight.setUV(_uv_rect.x + _uv_rect.z, _uv_rect.y + _uv_rect.w);

		glyphs.push_back(newGlyph);
	}

	/**
	 *	Draws all triangles of all spritebatches
	 */
	void SpriteBatch::RenderBatches()
	{
		glBindVertexArray(vao);

		for (int i = 0; i < (int)render_batches.size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, render_batches[i].texture);
			glDrawArrays(GL_TRIANGLES, render_batches[i].offset, render_batches[i].num_vertices);
		}
		glBindVertexArray(0);
	}

	/**
	 *	Makes renderable batches of glyphs
	 */
	void SpriteBatch::CreateRenderBatches()
	{
		std::vector<Vertex> vertices;

		vertices.resize(glyphs.size() * 6);

		if (glyphs.empty())
		{
			return;
		}
		// current vertex
		int cv = 0;
		render_batches.emplace_back(cv, 6, glyphs[0]->texture);

		// set vertices
		vertices[cv++] = glyphs[0]->topLeft;
		vertices[cv++] = glyphs[0]->bottomLeft;
		vertices[cv++] = glyphs[0]->bottomRight;
		vertices[cv++] = glyphs[0]->bottomRight;
		vertices[cv++] = glyphs[0]->topRight;
		vertices[cv++] = glyphs[0]->topLeft;

		for (int cg = 1; cg < (int)glyphs.size(); cg++)
		{
			if (glyphs[cg]->texture != glyphs[cg - 1]->texture)
			{
				render_batches.emplace_back(cv, 6, glyphs[cg]->texture);
			}
			else
			{
				render_batches.back().num_vertices += 6;
			}
			vertices[cv++] = glyphs[cg]->topLeft;
			vertices[cv++] = glyphs[cg]->bottomLeft;
			vertices[cv++] = glyphs[cg]->bottomRight;
			vertices[cv++] = glyphs[cg]->bottomRight;
			vertices[cv++] = glyphs[cg]->topRight;
			vertices[cv++] = glyphs[cg]->topLeft;
		}

		if (vbo == 0)
		{
			glGenBuffers(1, &vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/**
	 *	Creates Vertex array and buffer objects
	 */
	void SpriteBatch::CreateVertexArray()
	{
		if (vao == 0)
		{
			glGenVertexArrays(1, &vao);
		}
		glBindVertexArray(vao);

		if (vbo == 0)
		{
			glGenBuffers(1, &vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// Position attrib pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		// Color attrib pointer	
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		// UV attrib pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindVertexArray(0);

	}
	/**
	 *	Sorts glyphs based on declared sort type
	 */
	void SpriteBatch::SortGlyphs()
	{
		if (glyphs.size() <= 0)
		{
			return;
		}
		switch (sort_type)
		{
		case GlyphSortType::NONE:
			//dont sort
			break;
		case GlyphSortType::FRONT_TO_BACK:
			std::stable_sort(glyphs.begin(), glyphs.end(),
				[](const Glyph* a, const Glyph* b) -> bool
				{
					return a->depth < b->depth;
				});
			break;
		case GlyphSortType::BACK_TO_FRONT:
			std::stable_sort(glyphs.begin(), glyphs.end(),
				[](const Glyph* a, const Glyph* b) -> bool
				{
					return a->depth > b->depth;
				});
			break;
		case GlyphSortType::TEXTURE:
			std::stable_sort(glyphs.begin(), glyphs.end(),
				[](const Glyph* a, const Glyph* b) -> bool
				{
					// Text will render in front of everything
					return a->texture > b->texture;
				});
			break;
		default:
			break;
		}
	}

}
