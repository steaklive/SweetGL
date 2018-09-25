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
