#include <graphics/GLSLProgram.h>


namespace k2d
{

	GLSLProgram::GLSLProgram() :
		num_attributes(0),
		program_id(0),
		vertex_shader_id(0),
		fragment_shader_id(0)
	{

	}

	GLSLProgram::~GLSLProgram()
	{

	}

	bool GLSLProgram::CompileShaders(const std::string& _vertex_shader_file_path, const std::string& _fragment_shader_file_path)
	{
		program_id = glCreateProgram();

		vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
		if (vertex_shader_id == 0)
		{
			KUSI_ERROR("Vertex shader failed to be created!");
			return false;
		}
		fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
		if (fragment_shader_id == 0)
		{
			KUSI_ERROR("Fragment shader failed to be created!");
			return false;
		}

		CompileShader(_vertex_shader_file_path, vertex_shader_id);
		CompileShader(_fragment_shader_file_path, fragment_shader_id);

		return true;
	}

	bool GLSLProgram::LinkShaders()
	{
		glAttachShader(program_id, vertex_shader_id);
		glAttachShader(program_id, fragment_shader_id);

		glLinkProgram(program_id);

		GLint isLinked = 0;
		glGetProgramiv(program_id, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> errorLog(maxLength);
			glGetShaderInfoLog(program_id, maxLength, &maxLength, &errorLog[0]);

			glDeleteProgram(program_id);
			glDeleteShader(vertex_shader_id);
			glDeleteShader(fragment_shader_id);


			std::printf("%s\n", &errorLog[0]);
			KUSI_ERROR("Shaders failed to link!");
		}

		glDetachShader(program_id, vertex_shader_id);
		glDetachShader(program_id, fragment_shader_id);
		glDeleteShader(vertex_shader_id);
		glDeleteShader(fragment_shader_id);

		return true;
	}

	void GLSLProgram::AddAttribute(const std::string& attributeName)
	{
		glBindAttribLocation(program_id, num_attributes, attributeName.c_str());
		num_attributes++;
	}

	GLint GLSLProgram::GetUniformLocation(const std::string& uniformName)
	{
		GLint location = glGetUniformLocation(program_id, uniformName.c_str());
		if (location == GL_INVALID_INDEX)
		{
			KUSI_ERROR("Uniform " + uniformName + " not found in shader!");
		}
		return location;
	}

	void GLSLProgram::Set1i(GLint _value, const GLchar* _name)
	{
		Use();
		glUniform1i(GetUniformLocation(_name), _value);
		UnUse();
	}

	void GLSLProgram::Set1f(GLfloat _value, const GLchar* _name)
	{
		Use();
		glUniform1f(GetUniformLocation(_name), _value);
		UnUse();
	}

	void GLSLProgram::SetVec2f(glm::vec2 _value, const GLchar* _name)
	{
		Use();
		glUniform2fv(GetUniformLocation(_name), 1, glm::value_ptr(_value));
		UnUse();
	}

	void GLSLProgram::SetVec3f(glm::vec3 _value, const GLchar* _name)
	{
		Use();
		glUniform3fv(GetUniformLocation(_name), 1, glm::value_ptr(_value));
		UnUse();
	}

	void GLSLProgram::SetVec4f(glm::vec4 _value, const GLchar* _name)
	{
		Use();
		glUniform4fv(GetUniformLocation(_name), 1, glm::value_ptr(_value));
		UnUse();
	}

	void GLSLProgram::SetMat3fv(glm::mat3 _value, const GLchar* _name, GLboolean _transpose)
	{
		Use();
		glUniformMatrix3fv(GetUniformLocation(_name), 1, _transpose, glm::value_ptr(_value));
		UnUse();
	}

	void GLSLProgram::SetMat4fv(glm::mat4 _value, const GLchar* _name, GLboolean _transpose)
	{
		Use();
		glUniformMatrix4fv(GetUniformLocation(_name), 1, _transpose, glm::value_ptr(_value));
		UnUse();
	}

	void GLSLProgram::Use()
	{
		glUseProgram(program_id);
		for (int i = 0; i < num_attributes; i++)
		{
			glEnableVertexAttribArray(i);
		}
	}

	void GLSLProgram::UnUse()
	{
		glUseProgram(0);
		for (int i = 0; i < num_attributes; i++)
		{
			glDisableVertexAttribArray(i);
		}
	}

	void GLSLProgram::CompileShader(const std::string& filePath, GLuint& id)
	{
		std::ifstream vertexFile(filePath);
		if (vertexFile.fail())
		{
			perror(filePath.c_str());
			KUSI_ERROR("Failed to open " + filePath);
		}

		std::string fileContents = "";
		std::string line;

		while (std::getline(vertexFile, line))
		{
			fileContents += line + "\n";
		}

		vertexFile.close();

		const char* contentsPtr = fileContents.c_str();
		glShaderSource(id, 1, &contentsPtr, nullptr);

		glCompileShader(id);

		GLint success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxlength includes the NULL character
			std::vector<char> errorLog(maxLength);
			glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

			glDeleteShader(id);

			std::printf("%s\n", &errorLog[0]);
			KUSI_ERROR("Shader " + filePath + " failed to compile");
		}
	}

}