
#define WORK_GROUP_SIZE 256
#define GLM_FORCE_RADIANS

#include <iostream>
#include <chrono>

#include <vmath.h>

#include "SweetGL.h"
#include "SweetGLShader.h"
#include "ParticleGenerator.h"

class GPU_Particles_Demo : public SweetGL::Game
{

public:
	GPU_Particles_Demo() 
	{
	}

protected:
	void Initialize()
	{
		SweetGL::Game::Initialize();
		mGameInfo.title = "SweetGL - GPU Particles Demo";
	}

	virtual void Setup() override;
	virtual void Draw(double currentTime) override;
	//virtual void OnKeyPressed(int key, int action) override;

private:
	ParticleGenerator  mParticleGenerator;

	GLuint  mVertexUVBufferID;
	GLuint  mVertexArrayID;

	GLuint mComputeProgram, mShadingProgram;

	struct
	{
		GLuint dt;
		GLuint attractorPos;
		GLuint maxParticles = 10000;

	} ComputeUniforms;

	struct
	{
		GLuint projMatrix;
		GLuint viewMatrix;
		GLuint camPos;

	} GeometryUniforms;

	struct 
	{
		GLfloat time;
		GLuint texture;

	} FragmentUniforms;

	struct
	{
		float depth = 50.0f;
		vmath::vec4 position;
	
	} ParticleAttractor;

	struct
	{
		vmath::vec3 position = vmath::vec3(0.0f, 0.0f, 0.0f);
		float fov = 45.0f;
		float nearPlane = 2.0f;
		float farPlane = 300.0f;

		vmath::mat4     viewMatrix;
		vmath::mat4     projMatrix;

	} CameraData;

	void LoadShaders();
	void DrawImGUI();
	void UpdateAttractor();
};

