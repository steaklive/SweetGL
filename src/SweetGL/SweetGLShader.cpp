#define _CRT_SECURE_NO_WARNINGS 1

#include "GL/gl3w.h"
#include <cstdio>

namespace Shader
{
	extern GLuint LoadShader(const char * filename, GLenum shaderType, bool onError)
		{
			GLuint result = 0;
			FILE * fp;
			size_t filesize;
			char * data;

			fp = fopen(filename, "rb");

			if (!fp)
				return 0;

			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			data = new char[filesize + 1];

			if (!data) goto FailToAllocateData;

			fread(data, 1, filesize, fp);
			data[filesize] = 0;
			fclose(fp);

			result = glCreateShader(shaderType);

			if (!result) goto FailToAllocateShader;

			glShaderSource(result, 1, &data, NULL);

			delete[] data;

			glCompileShader(result);

			if (onError)
			{
				GLint status = 0;
				glGetShaderiv(result, GL_COMPILE_STATUS, &status);

				if (!status)
				{
					char buffer[4096];
					glGetShaderInfoLog(result, 4096, NULL, buffer);
					#ifdef _WIN32
					OutputDebugStringA(filename);
					OutputDebugStringA(":");
					OutputDebugStringA(buffer);
					OutputDebugStringA("\n");
					#else
					fprintf(stderr, "%s: %s\n", filename, buffer);
					#endif
					goto FailToCompileShader;
				}
			}

			return result;

			FailToCompileShader: glDeleteShader(result);

			FailToAllocateShader:;

			FailToAllocateData: return result;
		}
}

namespace ShaderProgram
{
	GLuint LinkShaders(const GLuint * shaders, int shaderCount, bool isShaderDeleted, bool onError)
		{
			int i;

			GLuint program;

			program = glCreateProgram();

			for (i = 0; i < shaderCount; i++)
			{
				glAttachShader(program, shaders[i]);
			}

			glLinkProgram(program);

			if (onError)
			{
				GLint status;
				glGetProgramiv(program, GL_LINK_STATUS, &status);

				if (!status)
				{
					char buffer[4096];
					glGetProgramInfoLog(program, 4096, NULL, buffer);
#ifdef _WIN32
					OutputDebugStringA(buffer);
					OutputDebugStringA("\n");
#endif
					glDeleteProgram(program);
					return 0;
				}
			}

			if (isShaderDeleted)
			{
				for (i = 0; i < shaderCount; i++)
				{
					glDeleteShader(shaders[i]);
				}
			}

			return program;
		}
}
