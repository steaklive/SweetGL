#pragma once

#include <vmath.h>

#include <SweetGL.h>
#include <SweetGLModel.h>
#include <SweetGLShader.h>


const int OBJECT_COUNT = 5;

class DOF_Demo : public SweetGL::Game
{
public:
	DOF_Demo()
		:
		mDisplayProgram(0),
		mFilterProgram(0),
		mViewProgram(0),
		mPaused(false),
		mFocalDistance(40.0f),
		mFocalDepth(50.0f)
	{
	}

protected:
	void Initialize()
	{

		SweetGL::Game::Initialize();
		mGameInfo.title = "SweetGL - DOF Demo";
	}

	virtual void Setup() override;
	virtual void Draw(double currentTime) override;

private:

	void RenderScene(double currentTime);
	void LoadShaders();
	void DrawImGUI();

	GLuint          mViewProgram;
	GLuint          mFilterProgram;
	GLuint          mDisplayProgram;

	struct
	{
		struct
		{
			GLint   mFocalDistance;
			GLint   mFocalDepth;
		} dof;
		struct
		{
			GLint   mModelViewMatrix;
			GLint   mProjMatrix;
			GLint   mFullShading;
			GLint   mAlbedo;
		} view;
	} uniforms;

	GLuint          mDepthFBO;
	GLuint          mDepthTexture;
	GLuint          mColorTexture;
	GLuint          mTempTexture;

	struct
	{
		SweetGLModel    mObject;
		vmath::mat4     mModelMatrix;
		vmath::vec4     mAlbedo;
	} objects[OBJECT_COUNT];


	vmath::mat4     mCameraViewMatrix;
	vmath::mat4     mCameraProjMatrix;

	GLuint          mQuadVAO;

	bool mPaused;

	float          mFocalDistance;
	float          mFocalDepth;

};