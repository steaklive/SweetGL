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

void APIENTRY SweetGL::Game::DebugCall(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	reinterpret_cast<Game*>(userParam)->OutputDebugMessage(source, type, id, severity, length, message);
}