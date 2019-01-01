#pragma once

#include <vmath.h>

#include <SweetGL.h>
#include <SweetGLModel.h>
#include <SweetGLShader.h>

#include <random>

class DeferredRendering_Demo : public SweetGL::Game
{
public:
	DeferredRendering_Demo()
		:
		mRenderProgram(0),
		mLightProgram(0),
		mIsPaused(false),
		ViewModes(SHADED),
		mNumberOfLights(25)
	{
	}

protected:
	void Initialize()
	{

		SweetGL::Game::Initialize();
		GameInfo.title = "SweetGL - Deferred Shading Demo";
	}

	virtual void Setup() override;
	virtual void Draw(double currentTime) override;
	virtual void Shutdown() override;

private:

	void LoadShaders();

	void SetupLights();
	void UpdateLights();
	void DrawImGUI();


	GLuint      mRenderTransformBuffer;
	GLuint      mLightBuffer;
	GLuint      mGbuffer;
	GLuint      mGbufferTextures[3];
	
	GLuint      mQuadVAO;

	struct
	{
		SweetGLModel model;
		GLuint modelMatrix;

	} object;

	GLuint      mRenderProgram;
	GLuint      mDebugVisualizationProgram;
	GLuint      mLightProgram;

	GLint       mDebugVisualizationMode;
	int		    mNumberOfLights;

	enum Modes
	{
		SHADED,
		NORMALS,
		COORDS,
		DIFFUSE,

	} ViewModes;

	struct Light
	{
		vmath::vec4         position;
		vmath::vec4         color;
		vmath::vec4			padding;
	};

	struct 
	{
		GLint numberOfLights;
	} LightUniforms;

	bool        mIsPaused;

};