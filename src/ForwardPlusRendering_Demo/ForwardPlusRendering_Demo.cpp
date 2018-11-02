//*****************************************************//
//              'Forward+ Shading' Demo                //
//*****************************************************//

#include "ForwardPlusRendering_Demo.h"
#include <random>

const vmath::vec3 MinLightBoundarries(-5.0f, -5.0f, -25.0f);
const vmath::vec3 MaxLightBoundarries(5.0f, 10.0f, 25.0f);


void ForwardPlusRendering_Demo::Setup()
{
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);	

	GenerateFrameBuffers();

	CameraData.projMatrix = vmath::perspective(
		CameraData.fov,
		(float)mGameInfo.windowWidth / (float)mGameInfo.windowHeight,
		CameraData.nearPlane,
		CameraData.farPlane
	);

	CameraData.viewMatrix = vmath::lookat(
		CameraData.position,
		vmath::vec3(0.0f, -2.5f, 0.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f));

	CameraData.screenSize = vmath::vec2(mGameInfo.windowWidth, mGameInfo.windowHeight);


	mWorkGroupsX = (mGameInfo.windowWidth + (mGameInfo.windowWidth%TILE_SIZE))/TILE_SIZE;
	mWorkGroupsY = (mGameInfo.windowHeight + (mGameInfo.windowHeight%TILE_SIZE)) / TILE_SIZE;

	GLuint tilesCount = mWorkGroupsX * mWorkGroupsY;
	   
	//Load model
	object.model.LoadModel("media/objects/sponza/sponza.fbx");
	assert(object.model.meshes.size() != 0);
	object.modelMatrix = vmath::translate(0.0f, 0.0f, 0.0f) *vmath::rotate(-90.0f, vmath::vec3(0.0, 1.0, 0.0));

	//Load shaders
	LoadShaders();

	// Create light buffer
	glGenBuffers(1, &mLightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, mNumberOfLights * sizeof(Light), 0, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0 , mLightBuffer );
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	// Create visible light indices buffer
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mIndexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * mWorkGroupsX*mWorkGroupsY * MAX_LIGHTS_PER_TILE , NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mIndexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Generate lights
	SetupLights();

	// Load uniforms

	glUseProgram(mDepthProgram);
	glUniformMatrix4fv(glGetUniformLocation(mDepthProgram, "model"), 1, GL_FALSE, object.modelMatrix);

	glUseProgram(mDepthDebugProgram);
	glUniformMatrix4fv(glGetUniformLocation(mDepthDebugProgram, "model"), 1, GL_FALSE, object.modelMatrix);
	glUniform1f(glGetUniformLocation(mDepthDebugProgram, "near"), CameraData.nearPlane);
	glUniform1f(glGetUniformLocation(mDepthDebugProgram, "far"), CameraData.farPlane);


	glUseProgram(mLightCullingProgram);
	glUniform1i(glGetUniformLocation(mLightCullingProgram, "lightCount"), mNumberOfLights);
	glUniform2fv(glGetUniformLocation(mLightCullingProgram, "screenSize"), 1, CameraData.screenSize);


	glUseProgram(mLightProgram);
	glUniformMatrix4fv(glGetUniformLocation(mLightProgram, "model"), 1, GL_FALSE, object.modelMatrix);
	glUniform1i(glGetUniformLocation(mLightProgram, "numberOfTilesX"), mWorkGroupsX);
	glUniform1i(glGetUniformLocation(mLightProgram, "doLightDebug"), 0);
	glUniform3fv(glGetUniformLocation(mLightProgram, "viewPosition"), 1, CameraData.position);

	
}

void ForwardPlusRendering_Demo::LoadShaders()
{
	if (mLightProgram)
		glDeleteProgram(mLightProgram);
	if (mRenderProgram)
		glDeleteProgram(mRenderProgram);
	if (mDepthProgram)
		glDeleteProgram(mDepthProgram);
	if (mLightCullingProgram)
		glDeleteProgram(mLightCullingProgram);
	if (mDepthDebugProgram)
		glDeleteProgram(mDepthDebugProgram);

	GLuint vs, fs, cs;

	// Depth Shader
	vs = Shader::LoadShader("media/shaders/forwardplusshading/depth.vs.glsl", GL_VERTEX_SHADER);
	fs = Shader::LoadShader("media/shaders/forwardplusshading/depth.fs.glsl", GL_FRAGMENT_SHADER);
	mDepthProgram = glCreateProgram();
	glAttachShader(mDepthProgram, vs);
	glAttachShader(mDepthProgram, fs);
	glLinkProgram(mDepthProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);

	// Depth  debug Shader
	vs = Shader::LoadShader("media/shaders/forwardplusshading/depth_debug.vert.glsl", GL_VERTEX_SHADER);
	fs = Shader::LoadShader("media/shaders/forwardplusshading/depth_debug.frag.glsl", GL_FRAGMENT_SHADER);
	mDepthDebugProgram = glCreateProgram();
	glAttachShader(mDepthDebugProgram, vs);
	glAttachShader(mDepthDebugProgram, fs);
	glLinkProgram(mDepthDebugProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);



	// Light culling shader
	cs = Shader::LoadShader("media/shaders/forwardplusshading/lightculling.cs.glsl", GL_COMPUTE_SHADER);
	mLightCullingProgram = glCreateProgram();
	glAttachShader(mLightCullingProgram, cs);
	glLinkProgram(mLightCullingProgram);
	glDeleteShader(cs);


	// Light calculation shader
	vs = Shader::LoadShader("media/shaders/forwardplusshading/lighting.vs.glsl", GL_VERTEX_SHADER);
	fs = Shader::LoadShader("media/shaders/forwardplusshading/lighting.fs.glsl", GL_FRAGMENT_SHADER);
	mLightProgram = glCreateProgram();
	glAttachShader(mLightProgram, vs);
	glAttachShader(mLightProgram, fs);
	glLinkProgram(mLightProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);

	// Final rendering shader
	vs = Shader::LoadShader("media/shaders/forwardplusshading/render.vs.glsl", GL_VERTEX_SHADER);
	fs = Shader::LoadShader("media/shaders/forwardplusshading/render.fs.glsl", GL_FRAGMENT_SHADER);
	mRenderProgram = glCreateProgram();
	glAttachShader(mRenderProgram, vs);
	glAttachShader(mRenderProgram, fs);
	glLinkProgram(mRenderProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);

}

void ForwardPlusRendering_Demo::UpdateSSBO()
{
	GLuint workgroup_x = (mGameInfo.windowWidth + (mGameInfo.windowWidth % TILE_SIZE)) / TILE_SIZE;
	GLuint workgroup_y = (mGameInfo.windowHeight + (mGameInfo.windowHeight % TILE_SIZE)) / TILE_SIZE;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mIndexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * workgroup_x * workgroup_y * MAX_LIGHTS_PER_TILE,	NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ForwardPlusRendering_Demo::UpdateLights()
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

void ForwardPlusRendering_Demo::SetupLights()
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

		light.paddingAndRadius = vmath::vec4(0, 0, 0, 8.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ForwardPlusRendering_Demo::Draw(double currentTime)
{
	static const GLuint uint_zeros[] = { 0, 0, 0, 0 };
	static const GLfloat float_zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const GLfloat float_ones[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	
	int i, j;
	static double last_time = 0.0;
	static double total_time = 0.0;
	
	total_time += (currentTime - last_time);
	last_time = currentTime;
	
	UpdateLights();
	UpdateSSBO();

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	CameraData.projMatrix = vmath::perspective(
		CameraData.fov,
		(float)mGameInfo.windowWidth / (float)mGameInfo.windowHeight,
		CameraData.nearPlane,
		CameraData.farPlane
	);
	
	CameraData.viewMatrix = vmath::lookat(
		CameraData.position,
		vmath::vec3(0.0f, -2.5f, 0.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f));

	CameraData.screenSize = vmath::vec2(mGameInfo.windowWidth, mGameInfo.windowHeight);

	object.modelMatrix = vmath::translate(0.0f, 0.0f, 0.0f) *vmath::rotate(-90.0f, vmath::vec3(0.0, 1.0, 0.0));


	#pragma region DEPTH_FBO
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mDepthMapFBO);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);

		////update depth uniforms
		//glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);

		glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset(4.0f, 4.0f);

		glUseProgram(mDepthProgram);
		static const GLenum buffs[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffs);
		glClearBufferfv(GL_COLOR, 0, float_zeros);
		glClearBufferfv(GL_DEPTH, 0, float_ones);

		glUniformMatrix4fv(glGetUniformLocation(mDepthProgram, "projection"), 1, GL_FALSE, CameraData.projMatrix);
		glUniformMatrix4fv(glGetUniformLocation(mDepthProgram, "view"), 1, GL_FALSE, CameraData.viewMatrix);
		glUniformMatrix4fv(glGetUniformLocation(mDepthProgram, "model"), 1, GL_FALSE, object.modelMatrix);

		object.model.Draw(mDepthProgram);

		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	#pragma endregion

	if (ViewModes == DEPTH)
	{
		#pragma region  DEBUG_DEPTH

		//Depth debug
		glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(mDepthDebugProgram);
		glUniformMatrix4fv(glGetUniformLocation(mDepthDebugProgram, "projection"), 1, GL_FALSE, CameraData.projMatrix);
		glUniformMatrix4fv(glGetUniformLocation(mDepthDebugProgram, "view"), 1, GL_FALSE, CameraData.viewMatrix);

		object.model.Draw(mDepthDebugProgram);

		#pragma endregion
	}
	else
	{
		#pragma region LIGHT_CULLING_COMPUTE
		{
			glDepthFunc(GL_EQUAL);
			glClear(GL_COLOR_BUFFER_BIT);

			mWorkGroupsX = (mGameInfo.windowWidth + (mGameInfo.windowWidth%TILE_SIZE)) / TILE_SIZE;
			mWorkGroupsY = (mGameInfo.windowHeight + (mGameInfo.windowHeight%TILE_SIZE)) / TILE_SIZE;

			glUseProgram(mLightCullingProgram);

			glUniformMatrix4fv(glGetUniformLocation(mLightCullingProgram, "projection"), 1, GL_FALSE, CameraData.projMatrix);
			glUniformMatrix4fv(glGetUniformLocation(mLightCullingProgram, "view"), 1, GL_FALSE, CameraData.viewMatrix);

			// Bind shader storage buffer objects for the light and indice buffers
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mLightBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mIndexBuffer);

			// Bind depth map texture to texture location 4 (which will not be used by any model texture)
			glActiveTexture(GL_TEXTURE4);
			glUniform1i(glGetUniformLocation(mLightCullingProgram, "depthMap"), 4);
			glBindTexture(GL_TEXTURE_2D, mDepthMap);


			// Dispatch the compute shader, using the workgroup values calculated earlier
			glDispatchCompute(mWorkGroupsX, mWorkGroupsY, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			// Unbind the depth map
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
#pragma endregion

		#pragma region RENDER_FBO
		{
			glDepthFunc(GL_LESS);

			glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBO);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(mLightProgram);
			glUniform3fv(glGetUniformLocation(mLightProgram, "viewPosition"), 1, CameraData.position);
			glUniformMatrix4fv(glGetUniformLocation(mLightProgram, "projection"), 1, GL_FALSE, CameraData.projMatrix);
			glUniformMatrix4fv(glGetUniformLocation(mLightProgram, "view"), 1, GL_FALSE, CameraData.viewMatrix);
			glUniformMatrix4fv(glGetUniformLocation(mLightProgram, "model"), 1, GL_FALSE, object.modelMatrix);

			if (ViewModes == LIGHT)
			{
				glUniform1i(glGetUniformLocation(mLightProgram, "doLightDebug"), 1);
			}
			else glUniform1i(glGetUniformLocation(mLightProgram, "doLightDebug"), 0);

			object.model.Draw(mLightProgram);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		#pragma endregion
		
		#pragma region RENDER_QUAD
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor
			glUseProgram(mRenderProgram);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mColorBuffer);
			
			mQuad->Render();

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		}
		#pragma endregion
	}
	

	glDisable(GL_DEPTH_TEST);


	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	DrawImGUI();
}

void ForwardPlusRendering_Demo::GenerateFrameBuffers()
{
	// Depth buffer 
	glGenFramebuffers(1, &mDepthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mDepthMapFBO);

	glGenTextures(1, &mDepthMap);
	glBindTexture(GL_TEXTURE_2D, mDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mGameInfo.windowWidth, mGameInfo.windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthMap, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);


	// Output buffer + color
	glGenFramebuffers(1, &mRenderFBO);
	
	glGenTextures(1, &mColorBuffer);
	glBindTexture(GL_TEXTURE_2D, mColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mGameInfo.windowWidth, mGameInfo.windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// render depth
	glGenRenderbuffers(1, &mRenderDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, mRenderDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mGameInfo.windowWidth, mGameInfo.windowHeight);
	
	glBindFramebuffer(GL_FRAMEBUFFER, mRenderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderDepthBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void ForwardPlusRendering_Demo::Shutdown()
{
	glDeleteFramebuffers(1, &mRenderFBO);
	glDeleteFramebuffers(1, &mDepthMapFBO);
	glDeleteBuffers(1, &mLightBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteBuffers(1, &mRenderDepthBuffer);
	glDeleteBuffers(1, &mColorBuffer);

	glDeleteTextures(1, &mDepthMap);

	glDeleteProgram(mRenderProgram);
	glDeleteProgram(mLightProgram);
	glDeleteProgram(mDepthProgram);
	glDeleteProgram(mColorProgram);
	glDeleteProgram(mDepthDebugProgram);
	glDeleteProgram(mLightCullingProgram);
}


void ForwardPlusRendering_Demo::DrawImGUI()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("SweetGL - Forward+ Shading");

		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char *)glGetString(GL_VENDOR));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char *)glGetString(GL_VERSION));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char *)glGetString(GL_RENDERER));
		ImGui::Separator();

		ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();

		ImGui::Text("Tile size: %ix%i", TILE_SIZE, TILE_SIZE);
		ImGui::Text("Max lights per tile: %i", MAX_LIGHTS_PER_TILE);

		if (ImGui::CollapsingHeader("View Mode"))
		{

			const char* items[] = { "Shaded", "Depth", "Light Debug" };
			static int item_current = 0;
			ImGui::Combo("Mode", &item_current, items, IM_ARRAYSIZE(items));

			ViewModes = static_cast<Modes>(item_current);

		}

		// here i have some bugs when debugging.
		// prob this is not the proper way to change num of lights dynamically...
		if (ImGui::SliderInt("Lights count", &mNumberOfLights, 1, 130))
		{
			SetupLights();
			glUseProgram(mLightCullingProgram);
			glUniform1i(glGetUniformLocation(mLightCullingProgram, "lightCount"), mNumberOfLights);
		}

		if (ImGui::Button("Recalculate lights"))
		{
			UpdateLights();
			SetupLights();
			glUseProgram(mLightCullingProgram);
			glUniform1i(glGetUniformLocation(mLightCullingProgram, "lightCount"), mNumberOfLights);
		}
		//ImGui::ListBox

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

DECLARE_MAIN(ForwardPlusRendering_Demo)
