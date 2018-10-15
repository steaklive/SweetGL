//*****************************************************//
//              'Deferred Shading' Demo                //
//*****************************************************//

#include "DeferredRendering_Demo.h"
#include <random>

const vmath::vec3 MinLightBoundarries (-5.0f, -5.0f, -20.0f);
const vmath::vec3 MaxLightBoundarries (5.0f, 10.0f, 20.0f);

void DeferredRendering_Demo::Setup()
{
	glGenFramebuffers(1, &mGbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mGbuffer);

	glGenTextures(3, mGbufferTextures);
	glBindTexture(GL_TEXTURE_2D, mGbufferTextures[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, mGbufferTextures[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, mGbufferTextures[2]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 2048, 2048);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mGbufferTextures[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mGbufferTextures[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mGbufferTextures[2], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &mQuadVAO);
	glBindVertexArray(mQuadVAO);

	object.model.LoadModel("media/objects/sponza/sponza.fbx");
	assert(object.model.meshes.size()!=0);


	LoadShaders();

	glGenBuffers(1, &mLightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLightBuffer);
	
	SetupLights();

	glGenBuffers(1, &mRenderTransformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, mRenderTransformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(vmath::mat4), NULL, GL_DYNAMIC_DRAW); // 3 because we need: proj, view, model matrices. Better be divided separetely...

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void DeferredRendering_Demo::LoadShaders()
{
	if (mLightProgram)
		glDeleteProgram(mLightProgram);
	if (mRenderProgram)
		glDeleteProgram(mRenderProgram);


	GLuint vs, fs;

	vs = Shader::LoadShader("media/shaders/deferredshading/render.vs.glsl", GL_VERTEX_SHADER);
	fs = Shader::LoadShader("media/shaders/deferredshading/render.fs.glsl", GL_FRAGMENT_SHADER);

	mRenderProgram = glCreateProgram();
	glAttachShader(mRenderProgram, vs);
	glAttachShader(mRenderProgram, fs);
	glLinkProgram(mRenderProgram);

	glDeleteShader(vs);
	glDeleteShader(fs);

	vs = Shader::LoadShader("media/shaders/deferredshading/light.vs.glsl", GL_VERTEX_SHADER);
	fs = Shader::LoadShader("media/shaders/deferredshading/light.fs.glsl", GL_FRAGMENT_SHADER);

	mLightProgram = glCreateProgram();
	glAttachShader(mLightProgram, vs);
	glAttachShader(mLightProgram, fs);
	glLinkProgram(mLightProgram);

	glUniform1i(LightUniforms.numberOfLights, mNumberOfLights);
	LightUniforms.numberOfLights = glGetUniformLocation(mLightProgram, "num_lights");

	glDeleteShader(fs);


	// for debugging
	fs = Shader::LoadShader("media/shaders/deferredshading/visualization.fs.glsl", GL_FRAGMENT_SHADER);

	mDebugVisualizationProgram = glCreateProgram();
	glAttachShader(mDebugVisualizationProgram, vs);
	glAttachShader(mDebugVisualizationProgram, fs);
	glLinkProgram(mDebugVisualizationProgram);

	mDebugVisualizationMode = glGetUniformLocation(mDebugVisualizationProgram, "vis_mode");

	glDeleteShader(vs);
	glDeleteShader(fs);

}

void DeferredRendering_Demo::UpdateLights()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLightBuffer);

	Light* lights = (Light *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);


	for (int i = 0; i < mNumberOfLights; i++)
	{
		Light& light = lights[i];

		float min = MinLightBoundarries[1];
		float max = MaxLightBoundarries[1];

		light.position += vmath::vec4(0, 1.0f, 0, 0);

		if (light.position[1] > MaxLightBoundarries[1])
		{
			light.position[1] -= (MaxLightBoundarries[1] - MinLightBoundarries[1]);
		}


	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DeferredRendering_Demo::SetupLights()
{
	glBufferData(GL_SHADER_STORAGE_BUFFER, mNumberOfLights * sizeof(Light), NULL, GL_DYNAMIC_DRAW);

	if (mLightBuffer == 0) return;

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(0, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLightBuffer);
	Light* lights = (Light *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < mNumberOfLights; i++)
	{
		Light& light = lights[i];

		light.position[0] = MinLightBoundarries[0] + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MaxLightBoundarries[0] - MinLightBoundarries[0])));
		light.position[1] = MinLightBoundarries[1] + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MaxLightBoundarries[1] - MinLightBoundarries[1])));
		light.position[2] = MinLightBoundarries[2] + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (MaxLightBoundarries[2] - MinLightBoundarries[2])));
		light.position[3] = 1.0f;


		light.color = vmath::vec4(
			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			1.0f
		);

		light.padding = vmath::vec4(0, 0, 0, 0);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DeferredRendering_Demo::Draw(double currentTime)
{
	static const GLuint uint_zeros[] = { 0, 0, 0, 0 };
	static const GLfloat float_zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const GLfloat float_ones[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	static const GLenum draw_buffers[] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1 
	};

	int i, j;
	static double last_time = 0.0;
	static double total_time = 0.0;

	total_time += (currentTime - last_time);
	last_time = currentTime;

	UpdateLights();

	glBindFramebuffer(GL_FRAMEBUFFER, mGbuffer);
	glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);
	glDrawBuffers(2, draw_buffers);
	glClearBufferuiv(GL_COLOR, 0, uint_zeros);
	glClearBufferuiv(GL_COLOR, 1, uint_zeros);
	glClearBufferfv(GL_DEPTH, 0, float_ones);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mRenderTransformBuffer);
	// we only need 3 matrices (projection, view, model), so we will keep them in one place... Although it's a bad approach!
	vmath::mat4 * matrices = reinterpret_cast<vmath::mat4 *>(glMapBufferRange(GL_UNIFORM_BUFFER, 0,	3 * sizeof(vmath::mat4),GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

	//proj matrix
	matrices[0] = vmath::perspective(60.0f,
		(float)mGameInfo.windowWidth / (float)mGameInfo.windowHeight,
		0.1f,
		1000.0f);

	//view matrix
	vmath::vec3 eye_pos = vmath::vec3(0.0f, -2.5f, -5.0f);
	matrices[1] = vmath::lookat(eye_pos,
		vmath::vec3(0.0f, -2.5f, 0.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f));

	//model matrix
	matrices[2] = vmath::translate(0.0f, 0.0f, 0.0f) *vmath::rotate(-90.0f, vmath::vec3(0.0, 1.0, 0.0));
	glUnmapBuffer(GL_UNIFORM_BUFFER);


	glUseProgram(mRenderProgram);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	object.model.Draw(mRenderProgram);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);
	glDrawBuffer(GL_BACK);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGbufferTextures[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGbufferTextures[1]);

	if (ViewModes == SHADED)
	{
		glUseProgram(mLightProgram);

		glUniform1i(LightUniforms.numberOfLights, mNumberOfLights);
		LightUniforms.numberOfLights = glGetUniformLocation(mLightProgram, "num_lights");

	}
	else
	{
		glUseProgram(mDebugVisualizationProgram);
		glUniform1i(mDebugVisualizationMode, ViewModes);
	}

	glDisable(GL_DEPTH_TEST);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mLightBuffer);
	
	glBindVertexArray(mQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DrawImGUI();
}

void DeferredRendering_Demo::Shutdown()
{
	glDeleteTextures(3, &mGbufferTextures[0]);
	glDeleteFramebuffers(1, &mGbuffer);
	glDeleteProgram(mRenderProgram);
	glDeleteProgram(mLightProgram);
}

void DeferredRendering_Demo::DrawImGUI()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("SweetGL - Deferred Shading");

		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char *)glGetString(GL_VENDOR));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char *)glGetString(GL_VERSION));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char *)glGetString(GL_RENDERER));
		ImGui::Separator();

		ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();

		if (ImGui::CollapsingHeader("View Mode"))
		{

			const char* items[] = { "Shaded", "Normals", "Coordinates", "Albedo" };
			static int item_current = 0;
			ImGui::Combo("Mode", &item_current, items, IM_ARRAYSIZE(items));

			ViewModes = static_cast<Modes>(item_current);

		}

		ImGui::SliderInt("Lights count", &mNumberOfLights, 25, 500);
		
		if (ImGui::Button("Recalculate lights"))
		{
			SetupLights();
		}
		//ImGui::ListBox

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

DECLARE_MAIN(DeferredRendering_Demo)
