#ifndef SWEETGL_H
#define SWEETGL_H

#ifdef WIN32
#pragma once
#define _CRT_SECURE_NO_WARNINGS 1

#define WIN32_LEAN_AND_MEAN 1

#define NOMINMAX
#include <Windows.h>
#else
#include <unistd.h>
#define Sleep(t) sleep(t)
#endif

#include "GL/gl3w.h"

#define GLFW_NO_GLU 1
#define GLFW_INCLUDE_GLCOREARB 1

#include "GLFW/glfw3.h"

#include <stdio.h>
#include <string>
#include <math.h>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


namespace SweetGL
{
	class Game
	{
	public:
		Game() {}
		virtual ~Game() {}

		struct INFO
		{
			std::string title;

			int windowWidth;
			int windowHeight;
			int majorVersion;
			int minorVersion;
			int samples;

			union
			{
				struct
				{
					unsigned int    fullscreen : 1;
					unsigned int    vsync : 1;
					unsigned int    cursor : 1;
					unsigned int    stereo : 1;
					unsigned int    debug : 1;
					unsigned int    robust : 1;
				};
				unsigned int        all;
			} flags;
		};

		virtual void Run(SweetGL::Game* game)
		{
			bool running = true;

			mGame = game;

			if (!glfwInit())
			{
				fprintf(stderr, "Failed to initialize GLFW\n");
				return;
			}

			Initialize();

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, mGameInfo.majorVersion);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, mGameInfo.minorVersion);

			#ifndef _DEBUG
						if (info.flags.debug)
			#endif /* _DEBUG */
			{
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
			}

			if (mGameInfo.flags.robust)
			{
				glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
			}
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			glfwWindowHint(GLFW_SAMPLES, mGameInfo.samples);
			glfwWindowHint(GLFW_STEREO, mGameInfo.flags.stereo ? GL_TRUE : GL_FALSE);
			{
				mWindow = glfwCreateWindow(mGameInfo.windowWidth, mGameInfo.windowHeight, mGameInfo.title.c_str(), mGameInfo.flags.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
				if (!mWindow)
				{
					fprintf(stderr, "Failed to open window\n");
					return;
				}
			}

			glfwMakeContextCurrent(mWindow);

			//Callbacks for input and etc.
			glfwSetWindowSizeCallback		(mWindow, GLFWwindowResize);
			glfwSetKeyCallback				(mWindow, GLFWwindowKeyPress);
			glfwSetMouseButtonCallback		(mWindow, GLFWwindowMouseButtonPress);
			glfwSetCursorPosCallback		(mWindow, GLFWwindowMouseMove);
			glfwSetScrollCallback			(mWindow, GLFWwindowMouseWheel);

			if (!mGameInfo.flags.cursor)
			{
				glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}

			//Initialize GL
			gl3wInit();

			//Initialize ImGUI
			InitializeImGUI(mWindow);

			#ifdef _DEBUG
						fprintf(stderr, "VENDOR: %s\n", (char *)glGetString(GL_VENDOR));
						fprintf(stderr, "VERSION: %s\n", (char *)glGetString(GL_VERSION));
						fprintf(stderr, "RENDERER: %s\n", (char *)glGetString(GL_RENDERER));
			#endif

			if (mGameInfo.flags.debug)
			{
				if (gl3wIsSupported(4, 3))
				{
					glDebugMessageCallback((GLDEBUGPROC)DebugCall, this);
					glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				}
			}

			Setup();

			do
			{
				Draw(glfwGetTime());

				glfwSwapBuffers(mWindow);
				glfwPollEvents();

				
				running &= (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_RELEASE);
				running &= (glfwWindowShouldClose(mWindow) != GL_TRUE);
			} while (running);

			Shutdown();

			glfwDestroyWindow(mWindow);
			glfwTerminate();
		}
		virtual void Initialize()
		{
			mGameInfo.title = "SweetGL - Sample Window";
			mGameInfo.windowWidth = 960;
			mGameInfo.windowHeight = 640;

			mGameInfo.majorVersion = 4;
			mGameInfo.minorVersion = 4;

			mGameInfo.samples = 0;
			mGameInfo.flags.all = 0;
			mGameInfo.flags.cursor = 1;

			#ifdef _DEBUG
				mGameInfo.flags.debug = 1;
			#endif

		}
		virtual void Setup()
		{

		}
		virtual void Draw(double currentTime)
		{

		}
		virtual void Shutdown()
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		void SetWindowTitle(const char * title)
		{
			glfwSetWindowTitle(mWindow, title);
		}
		void GetMousePosition(int& x, int& y)
		{
			double dx, dy;
			glfwGetCursorPos(mWindow, &dx, &dy);

			x = static_cast<int>(floor(dx));
			y = static_cast<int>(floor(dy));
		}

		void InitializeImGUI(GLFWwindow* window)
		{
			// Setup Dear ImGui binding
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init(NULL);

			// Setup style
			ImGui::StyleSweetGLColor();
		}

		// Actions 
		virtual void OnWindowResize(int w, int h)
		{
			mGameInfo.windowWidth = w;
			mGameInfo.windowHeight = h;
		}
		virtual void OnKeyPressed(int key, int action)
		{

		}
		virtual void OnMouseClicked(int button, int action)
		{

		}
		virtual void OnMouseMoved(int x, int y)
		{

		}
		virtual void OnMouseWheel(int pos)
		{

		}

		virtual void OutputDebugMessage(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message)
		{
#ifdef _WIN32
			OutputDebugStringA(message);
			OutputDebugStringA("\n");
#endif /* _WIN32 */
		}


	private:
		static void APIENTRY DebugCall(GLenum source, GLenum type, GLuint id,GLenum severity,GLsizei length,const GLchar* message,GLvoid* userParam	);

	protected:

		static SweetGL::Game *mGame;
		INFO mGameInfo;
		GLFWwindow* mWindow;

		static void GLFWwindowResize(GLFWwindow* window, int w, int h)
		{
			mGame->OnWindowResize(w, h);
		}

		static void GLFWwindowKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			mGame->OnKeyPressed(key, action);
		}

		static void GLFWwindowMouseButtonPress(GLFWwindow* window, int button, int action, int mods)
		{
			mGame->OnMouseClicked(button, action);
		}

		static void GLFWwindowMouseMove(GLFWwindow* window, double x, double y)
		{
			mGame->OnMouseMoved(static_cast<int>(x), static_cast<int>(y));
		}

		static void GLFWwindowMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
		{
			mGame->OnMouseWheel(static_cast<int>(yoffset));
		}

		void SetVsync(bool enable)
		{
			mGameInfo.flags.vsync = enable ? 1 : 0;
			glfwSwapInterval((int)mGameInfo.flags.vsync);
		}
	};

}

#if defined _WIN32
#define DECLARE_MAIN(a)                             \
SweetGL::Game *game = 0;                            \
int CALLBACK WinMain(HINSTANCE hInstance,           \
                     HINSTANCE hPrevInstance,       \
                     LPSTR lpCmdLine,               \
                     int nCmdShow)                  \
{                                                   \
    a *game = new a;                                \
    game->Run(game);                                \
    delete game;                                    \
    return 0;                                       \
}
#endif

#endif // SWEETGL_H
