/////////////////////////////////////////////////////////////////////////////////////////////////
//																							   //
// 'SweetGL' is a simple OpenGL rendering framework written in C++ for educational purposes.   //
//																							   //
//	~ Gen Afanasev, 2018																	   //
//																							   //
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <SweetGL.h>
#include <GL/glext.h>

SweetGL::Game* SweetGL::Game::mGame = 0;


GL3WglProc sb6GetProcAddress(const char * funcname)
{
	return gl3wGetProcAddress(funcname);
}

int sb6IsExtensionSupported(const char * extname)
{
	GLint numExtensions;
	GLint i;

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	for (i = 0; i < numExtensions; i++)
	{
		const GLubyte * e = glGetStringi(GL_EXTENSIONS, i);
		if (!strcmp((const char *)e, extname))
		{
			return 1;
		}
	}

	return 0;
}

void APIENTRY SweetGL::Game::DebugCall(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	reinterpret_cast<Game*>(userParam)->OutputDebugMessage(source, type, id, severity, length, message);
}