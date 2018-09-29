#include "ParticleGenerator.h"

void ParticleGenerator::InitializeParticles() {

	Particle* particles = new Particle[mNumParticles];
	Distribute(particles);

	glGetError();

	glGenBuffers(1, &mBufferID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, mNumParticles * sizeof(Particle), particles, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mBufferID);

}

void ParticleGenerator::Distribute(Particle* particles) {
	float rndX, rndY, rndZ;
	auto fInitRadius = static_cast<float>(mRadius);

	std::mt19937 eng;
	std::uniform_real_distribution<float> dist(fInitRadius*(-1.f), fInitRadius);

	for (unsigned int i = 0; i < mNumParticles; ++i) {
		rndX = dist(eng);
		rndY = dist(eng);
		rndZ = dist(eng);
		particles[i].currPosition = vmath::vec4(rndX, rndY,rndZ, 1.f);
		particles[i].prevPosition = vmath::vec4(rndX, rndY,rndZ, 1.f);
	}
}

GLuint ParticleGenerator::GetParticleID() const {
	return mBufferID;
}

unsigned int ParticleGenerator::GetNumParticles() const {
	return mNumParticles;
}

void ParticleGenerator::Destroy() 
{
	if (!mBufferID) return;
	glGetError();
	glDeleteBuffers(1, &mBufferID);
}