//**********************************************//
// 'Depth of Field' Demo with Compute Shaders.	//
//**********************************************//

#include "DOF_Demo.h"

#define FBO_SIZE                4096
#define FRUSTUM_DEPTH           1000

static GLfloat backgroundColor[] = { 0.358f, 0.358f, 0.358f, 0.358f };


void DOF_Demo::Setup()
{
	LoadShaders();

	static const vmath::vec4 object_colors[] =
	{
		vmath::vec4(1.0f, 0.9f, 0.8f, 1.0f),
		vmath::vec4(0.2f, 0.8f, 1.0f, 1.0f),
		vmath::vec4(0.3f, 0.9f, 0.2f, 1.0f),
		vmath::vec4(0.8f, 0.4f, 0.9f, 1.0f),
		vmath::vec4(0.6f, 0.1f, 0.0f, 1.0f),
	};

	for (int i = 0; i < 5; i++)
	{
		objects[i].mObject.LoadModel("media/objects/bunny.fbx");
		objects[i].mAlbedo = object_colors[i];
	}

	glGenFramebuffers(1, &mDepthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mDepthFBO);

	glGenTextures(1, &mDepthTexture);
	glBindTexture(GL_TEXTURE_2D, mDepthTexture);
	glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, FBO_SIZE, FBO_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &mColorTexture);
	glBindTexture(GL_TEXTURE_2D, mColorTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

	glGenTextures(1, &mTempTexture);
	glBindTexture(GL_TEXTURE_2D, mTempTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &mQuadVAO);
	glBindVertexArray(mQuadVAO);

}

void DOF_Demo::Draw(double currentTime)
{
	static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	static double lastTime = 0.0;
	static double totalTime = 0.0;

	if (!mPaused)
		totalTime += (currentTime - lastTime);
	lastTime = currentTime;

	const float f = (float)totalTime + 30.0f;

	vmath::vec3 view_position = vmath::vec3(0.0f, 0.0f, 50.0f);

	mCameraProjMatrix = vmath::perspective(30.0f,
		(float)mGameInfo.windowWidth / (float)mGameInfo.windowHeight,
		2.0f,
		300.0f);

	mCameraViewMatrix = vmath::lookat(view_position,
		vmath::vec3(0.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f));

	objects[0].mModelMatrix = vmath::translate(5.0f, -1.0f, 20.0f) *
		vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
		vmath::translate(0.0f, 0.0f, 0.0f);

	objects[1].mModelMatrix = vmath::translate(1.0f, -1.0f, 0.0f) *
		vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
		vmath::translate(0.0f, 0.0f, 0.0f);

	objects[2].mModelMatrix = vmath::translate(-3.0f, -1.0f, -20.0f) *
		vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
		vmath::translate(0.0f, 0.0f, 0.0f);

	objects[3].mModelMatrix = vmath::translate(-7.0f, -1.0f, -40.0f) *
		vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
		vmath::translate(0.0f, 0.0f, 0.0f);

	objects[4].mModelMatrix = vmath::translate(-11.0f, -1.0f, -60.0f) *
		vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
		vmath::translate(0.0f, 0.0f, 0.0f);


	glEnable(GL_DEPTH_TEST);
	RenderScene(totalTime);

	glUseProgram(mFilterProgram);

	glBindImageTexture(0, mColorTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, mTempTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(mGameInfo.windowHeight, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindImageTexture(0, mTempTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, mColorTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glDispatchCompute(mGameInfo.windowWidth, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mColorTexture);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(mDisplayProgram);
	glUniform1f(uniforms.dof.mFocalDistance, mFocalDistance);
	glUniform1f(uniforms.dof.mFocalDepth, mFocalDepth);
	glBindVertexArray(mQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	DrawImGUI();

}

void DOF_Demo::RenderScene(double currentTime)
{
	static const GLfloat ones[] = { 1.0f };
	static const GLfloat zero[] = { 0.0f };
	static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };

	static const vmath::mat4 scale_bias_matrix =
		vmath::mat4(
			vmath::vec4(0.5f, 0.0f, 0.0f, 0.0f),
			vmath::vec4(0.0f, 0.5f, 0.0f, 0.0f),
			vmath::vec4(0.0f, 0.0f, 0.5f, 0.0f),
			vmath::vec4(0.5f, 0.5f, 0.5f, 1.0f)
		);

	glBindFramebuffer(GL_FRAMEBUFFER, mDepthFBO);

	glDrawBuffers(1, attachments);
	glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);
	glClearBufferfv(GL_COLOR, 0, backgroundColor);
	glClearBufferfv(GL_DEPTH, 0, ones);
	glUseProgram(mViewProgram);
	glUniformMatrix4fv(uniforms.view.mProjMatrix, 1, GL_FALSE, mCameraProjMatrix);

	glClearBufferfv(GL_DEPTH, 0, ones);


	for (int i = 0; i < OBJECT_COUNT; i++)
	{
		vmath::mat4& model_matrix = objects[i].mModelMatrix;
		glUniformMatrix4fv(uniforms.view.mModelViewMatrix, 1, GL_FALSE, mCameraViewMatrix * objects[i].mModelMatrix);
		glUniform3fv(uniforms.view.mAlbedo, 1, objects[i].mAlbedo);
		objects[0].mObject.Draw(mViewProgram);
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DOF_Demo::LoadShaders()
{
	GLuint shaders[4];

	shaders[0] = Shader::LoadShader("media/shaders/dof/render.vs.glsl", GL_VERTEX_SHADER);
	shaders[1] = Shader::LoadShader("media/shaders/dof/render.fs.glsl", GL_FRAGMENT_SHADER);

	if (mViewProgram)
		glDeleteProgram(mViewProgram);

	mViewProgram = ShaderProgram::LinkShaders(shaders, 2, true);

	uniforms.view.mProjMatrix = glGetUniformLocation(mViewProgram, "proj_matrix");
	uniforms.view.mModelViewMatrix = glGetUniformLocation(mViewProgram, "mv_matrix");
	uniforms.view.mFullShading = glGetUniformLocation(mViewProgram, "full_shading");
	uniforms.view.mAlbedo = glGetUniformLocation(mViewProgram, "diffuse_albedo");

	shaders[0] = Shader::LoadShader("media/shaders/dof/display.vs.glsl", GL_VERTEX_SHADER);
	shaders[1] = Shader::LoadShader("media/shaders/dof/display.fs.glsl", GL_FRAGMENT_SHADER);

	if (mDisplayProgram)
		glDeleteProgram(mDisplayProgram);

	mDisplayProgram = ShaderProgram::LinkShaders(shaders, 2, true);

	uniforms.dof.mFocalDistance = glGetUniformLocation(mDisplayProgram, "focal_distance");
	uniforms.dof.mFocalDepth = glGetUniformLocation(mDisplayProgram, "focal_depth");

	shaders[0] = Shader::LoadShader("media/shaders/dof/gensat.cs.glsl", GL_COMPUTE_SHADER);

	if (mFilterProgram)
		glDeleteProgram(mFilterProgram);

	mFilterProgram = ShaderProgram::LinkShaders(shaders, 1, true);
}

void DOF_Demo::DrawImGUI()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("SweetGL - Depth of Field");   

		ImGui::TextColored( ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char *)glGetString(GL_VENDOR));
		ImGui::TextColored( ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char *)glGetString(GL_VERSION));
		ImGui::TextColored( ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char *)glGetString(GL_RENDERER));
		ImGui::Separator();

		ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Checkbox("Pause", &mPaused);      
		ImGui::Separator();

		ImGui::ColorEdit3("Background color", (float*)&backgroundColor); 
		ImGui::SliderFloat("Focal distance", &mFocalDistance, 0.0f, 100.0f);  
		ImGui::SliderFloat("Focal depth", &mFocalDepth, 0.0f, 100.0f);

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	//int display_w, display_h;
	//glfwMakeContextCurrent(mWindow);
	//glfwGetFramebufferSize(mWindow, &display_w, &display_h);
	//glViewport(0, 0, display_w, display_h);
	//glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	//glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

DECLARE_MAIN(DOF_Demo)
