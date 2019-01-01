#ifndef __SWEETGLSHADER_H__
#define __SWEETGLSHADER_H__


	namespace Shader
	{

		GLuint LoadShader(const char * filename, GLenum shaderType = GL_FRAGMENT_SHADER,
		#ifdef _DEBUG
					bool onError = true);
		#else
					bool onError = false);
		#endif

		
		// utility uniform functions
		// ------------------------------------------------------------------------
		void SetBool(unsigned int ID, const std::string &name, bool value)
		{
			glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
		}
		// ------------------------------------------------------------------------
		void SetInt(unsigned int ID, const std::string &name, int value)
		{
			glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void SetFloat(unsigned int ID, const std::string &name, float value)
		{
			glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void SetVec2(unsigned int ID, const std::string &name, const vmath::vec2 &value)
		{
			glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
		}
		void SetVec2(unsigned int ID, const std::string &name, float x, float y) 
		{
			glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
		}
		// ------------------------------------------------------------------------
		void SetVec3(unsigned int ID, const std::string &name, const vmath::vec3 &value)
		{
			glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
		}
		void SetVec3(unsigned int ID, const std::string &name, float x, float y, float z) 
		{
			glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
		}
		// ------------------------------------------------------------------------
		void SetVec4(unsigned int ID, const std::string &name, const vmath::vec4 &value)
		{
			glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
		}
		void SetVec4(unsigned int ID, const std::string &name, float x, float y, float z, float w)
		{
			glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
		}
		// ------------------------------------------------------------------------
		void SetMat2(unsigned int ID, const std::string &name, const vmath::mat2 &mat)
		{
			glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void SetMat3(unsigned int ID, const std::string &name, const vmath::mat3 &mat) 
		{
			glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void SetMat4(unsigned int ID, const std::string &name, const vmath::mat4 &mat) 
		{
			glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}



	}
	namespace ShaderProgram
	{

		GLuint LinkShaders(const GLuint * shaders, int shaderCount, bool isShaderDeleted,
		#ifdef _DEBUG
					bool onError = true);
		#else
					bool onError = false);
		#endif

	}

#endif 
