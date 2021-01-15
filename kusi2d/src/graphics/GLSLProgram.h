#pragma once

#include <string>
#include <GL/glew.h>
#include <util/util.h>

#include <fstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace k2d
{
	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram();

		bool CompileShaders(const std::string& _vertex_shader_file_path, const std::string& _fragment_shader_file_path);

		bool LinkShaders();

		void AddAttribute(const std::string& _attribute_name);

		GLint GetUniformLocation(const std::string& _uniform_name);

		void Set1i(GLint _value, const GLchar* _name);
		void Set1f(GLfloat _value, const GLchar* _name);
		void SetVec2f(glm::vec2 _value, const GLchar* _name);
		void SetVec3f(glm::vec3 _value, const GLchar* _name);
		void SetVec4f(glm::vec4 _value, const GLchar* _name);

		void SetMat3fv(glm::mat3 _value, const GLchar* _name, GLboolean _transpose = GL_FALSE);
		void SetMat4fv(glm::mat4 _value, const GLchar* _name, GLboolean _transpose = GL_FALSE);

		void Use();
		void UnUse();

	private:
		int num_attributes;

		void CompileShader(const std::string& _file_path, GLuint& _id);

		GLuint program_id;

		GLuint vertex_shader_id;
		GLuint fragment_shader_id;
	};

}