#pragma once

#include "SweetGL.h"
#include "vmath.h"

#include <iostream>
#include <random>

struct Particle
{
	vmath::vec4 currPosition;
	vmath::vec4 prevPosition;
};

class ParticleGenerator
{
public:
	ParticleGenerator(): mBufferID(0), mNumParticles(10000), mRadius(10) 
	{
	}

	void InitializeParticles();
	GLuint GetParticleID() const;
	unsigned int GetNumParticles() const;

private:

	GLuint       mBufferID;
	unsigned int mNumParticles;
	int          mRadius;

	void Distribute(Particle* particles);
	void Destroy();

};
