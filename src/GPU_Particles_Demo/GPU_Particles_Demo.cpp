//*****************************************************//
// 'GPU Particles' Demo with Compute/Geometry Shaders  //
//*****************************************************//

#define PI 3.14159265358979323846f

#include "GPU_Particles_Demo.h"

#include "SweetGLModel.h"
#include <math.h>


unsigned int LoadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void GPU_Particles_Demo::Setup() {

	vmath::vec3 view_position = vmath::vec3(0.0f, 0.0f, 0.0f);

	CameraData.projMatrix = vmath::perspective(
		CameraData.fov,
		(float)mGameInfo.windowWidth / (float)mGameInfo.windowHeight,
		CameraData.nearPlane,
		CameraData.farPlane
	);

	CameraData.viewMatrix = vmath::lookat(
		view_position,
		vmath::vec3(0.0f, 0.0f, -1.0f),
		vmath::vec3(0.0f, 1.0f, 0.0f));

	// Generate Vertex Arrays and initialize particles
	mParticleGenerator.InitializeParticles();

	glGenVertexArrays(1, &mVertexArrayID);
	glBindVertexArray(mVertexArrayID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleGenerator.GetParticleID());
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, mParticleGenerator.GetParticleID());
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)0);
	glBindVertexArray(0);

	LoadShaders();

	// Depth test needs to be disabled for only rendering transparent particles. 
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void GPU_Particles_Demo::LoadShaders()
{
	GLuint shaders[4];

	shaders[0] = Shader::LoadShader("media/shaders/gpuparticles/particles.vs.glsl", GL_VERTEX_SHADER);
	shaders[1] = Shader::LoadShader("media/shaders/gpuparticles/particles.gs.glsl", GL_GEOMETRY_SHADER);
	shaders[2] = Shader::LoadShader("media/shaders/gpuparticles/particles.fs.glsl", GL_FRAGMENT_SHADER);

	if (mShadingProgram)
		glDeleteProgram(mShadingProgram);

	mShadingProgram = ShaderProgram::LinkShaders(shaders, 3, true);

	glUseProgram(mShadingProgram);

	glUniformMatrix4fv(GeometryUniforms.projMatrix, 1, GL_FALSE, CameraData.projMatrix);

	FragmentUniforms.texture = LoadTexture("media/textures/Particle.tga");

	GeometryUniforms.projMatrix =			glGetUniformLocation(mShadingProgram, "projMatrix");
	GeometryUniforms.camPos =				glGetUniformLocation(mShadingProgram, "camPos");
	GeometryUniforms.viewMatrix =			glGetUniformLocation(mShadingProgram, "viewMatrix");
	FragmentUniforms.time =					glGetUniformLocation(mShadingProgram, "time");
	FragmentUniforms.texture =			    glGetUniformLocation(mShadingProgram, "texture");


	shaders[0] = Shader::LoadShader("media/shaders/gpuparticles/particles.cs.glsl", GL_COMPUTE_SHADER);

	if (mComputeProgram) glDeleteProgram(mComputeProgram);

	mComputeProgram = ShaderProgram::LinkShaders(shaders, 1, true);

	glUseProgram(mComputeProgram);

	glUniform1i(ComputeUniforms.maxParticles, mParticleGenerator.GetNumParticles());

	ComputeUniforms.maxParticles  =		glGetUniformLocation(mComputeProgram, "maxParticles");
	ComputeUniforms.dt =				glGetUniformLocation(mComputeProgram, "frameTimeDiff");
	ComputeUniforms.attractorPos =		glGetUniformLocation(mComputeProgram, "attPos");

	glUseProgram(0);

}

void GPU_Particles_Demo::Draw(double currentTime) {

	UpdateAttractor();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, mGameInfo.windowWidth, mGameInfo.windowHeight);

	glBindVertexArray(mVertexArrayID);

	// Update compute program uniforms
	glUseProgram(mComputeProgram);

	// Load delta time
	glUniform1f(ComputeUniforms.dt, 0.01f);

	// Load attractor position
	glUniform4fv(ComputeUniforms.attractorPos, 1, ParticleAttractor.position);

	glDispatchCompute((mParticleGenerator.GetNumParticles() / WORK_GROUP_SIZE) + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);


	// Update shader program uniforms
	glUseProgram(mShadingProgram);

	// Load camera view matrix
	glUniformMatrix4fv(GeometryUniforms.viewMatrix, 1, GL_FALSE, CameraData.viewMatrix);

	// Load camera proj matrix
	glUniformMatrix4fv(GeometryUniforms.projMatrix, 1, GL_FALSE, CameraData.projMatrix);

	// Load camera pos
	glUniform4fv(GeometryUniforms.camPos, 1, vmath::vec4(CameraData.position[0], CameraData.position[1], CameraData.position[2], 1.0f));

	// Load current time
	glUniform1f(FragmentUniforms.time, currentTime);


	glDrawArrays(GL_POINTS, 0, mParticleGenerator.GetNumParticles());

	glBindVertexArray(0);
	glUseProgram(0);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DrawImGUI();
}

//========================================================//
// Credits for Particle Attractor to Stanislaw Eppinger   //
//========================================================//
void GPU_Particles_Demo::UpdateAttractor()
{
	vmath::vec3 view, h, v, pos, dir, attractor(0);

	auto width = static_cast<float>(mGameInfo.windowWidth);
	auto height = static_cast<float>(mGameInfo.windowHeight);

	float aspectRatio = width / height;

	//Compute the coordinate axes
	view	= vmath::vec3(0.0f, 0.0f, -1.0f); // view
	h		= vmath::vec3(1.0f, 0.0f, 0.0f);  // right vector
	v		= vmath::vec3(0.0f, 1.0f, 0.0f);  // up vector

	//Scale them
	float vLength = std::tan(CameraData.fov * PI / 360.f) * CameraData.nearPlane;

	v *= vLength;
	h *= vLength * aspectRatio;

	int xMousePos, yMousePos;
	GetMousePosition(xMousePos, yMousePos);

	float mouseX = (static_cast<float>(xMousePos) - (width / 2.0f)) / (width / 2.0f);	//Map the coordinate to [-1, 1]
	float mouseY = (static_cast<float>(yMousePos) - (height / 2.0f)) / (height / 2.0f); //Map the coordinate to [-1, 1]

	//Compute the intersection with the near plane
	pos = vmath::vec3(0) + view * CameraData.nearPlane + h * mouseX - v * mouseY;
	
	//Compute the direction of the ray
	dir = vmath::normalize(pos - CameraData.position);

	//Shoot attractor along the ray to the given depth
	attractor = pos + dir * ParticleAttractor.depth;

	//Update attractor
	ParticleAttractor.position = vmath::vec4(attractor, 1.0f);
}

void GPU_Particles_Demo::DrawImGUI()
{
	// Create a simple ImGUI config window.
	{
		ImGui::Begin("SweetGL - GPU particles");

		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Vendor: %s", (char *)glGetString(GL_VENDOR));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Version: %s", (char *)glGetString(GL_VERSION));
		ImGui::TextColored(ImVec4(0.4f, 0.4f, 1.0f, 1.00f), "Renderer: %s", (char *)glGetString(GL_RENDERER));
		ImGui::Separator();

		ImGui::Text("Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();

		ImGui::SliderFloat("Attractor Depth", &ParticleAttractor.depth, 0.0f, 100.0f);

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

DECLARE_MAIN(GPU_Particles_Demo)