#pragma once
#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE  16
#define GLM_ENABLE_EXPERIMENTAL

#include <vmath.h>

#include <SweetGL.h>
#include <SweetGLModel.h>
#include <SweetGLShader.h>
#include <SweetGLQuad.h>

#include <random>

class ForwardPlusRendering_Demo : public SweetGL::Game
{
public:
	ForwardPlusRendering_Demo()
		:
		mRenderProgram(0),
		mLightProgram(0),
		mLightCullingProgram(0),
		mDepthProgram(0),
		mDepthDebugProgram(0),

		ViewModes(SHADED),
		mNumberOfLights(35)
	{
	}

protected:
	void Initialize()
	{

		SweetGL::Game::Initialize();
		mGameInfo.title = "SweetGL - Forward+ Shading Demo";

	}

	virtual void Setup() override;
	virtual void Draw(double currentTime) override;
	virtual void Shutdown() override;

private:

	void LoadShaders();
	void GenerateFrameBuffers();

	void SetupLights();
	void UpdateLights();
	void UpdateSSBO();
	void DrawImGUI();
	

	GLuint		mDepthMapFBO;
	GLuint      mRenderFBO;

	GLuint		mRenderDepthBuffer;
	GLuint		mColorBuffer;
	GLuint      mLightBuffer;
	GLuint		mIndexBuffer;

	GLuint		mDepthMap;	

	std::unique_ptr<SweetGLQuad> mQuad = std::make_unique<SweetGLQuad>();

	struct
	{
		SweetGLModel model;
		vmath::mat4 modelMatrix;

	} object;

	struct
	{
		vmath::vec3 position = vmath::vec3(0.0f, -2.5f, -5.0f);
		float fov = 60.0f;
		float nearPlane = 0.5f;
		float farPlane = 300.0f;

		vmath::mat4     viewMatrix;
		vmath::mat4     projMatrix;
		vmath::vec2		screenSize;

	} CameraData;



	GLuint      mRenderProgram;
	GLuint      mDepthDebugProgram;
	GLuint      mLightProgram;

	GLuint		mLightCullingProgram;
	GLuint		mDepthProgram;
	GLuint      mColorProgram;

	int		    mNumberOfLights;

	GLuint mWorkGroupsX;
	GLuint mWorkGroupsY;

	enum Modes
	{
		SHADED,
		DEPTH,
		LIGHT
	} ViewModes;

	struct Light
	{
		vmath::vec4         position;
		vmath::vec4         color;
		vmath::vec4			paddingAndRadius;
	};

};