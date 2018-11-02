#ifndef SWEETGLQUAD_H
#define SWEETGLQUAD_H
#pragma once

#include "vmath.h"
#include "GL/gl3w.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;


class SweetGLQuad {

private:
	GLuint      mQuadVAO = 0;
	GLuint      mQuadVBO = 0;

public:
	SweetGLQuad() {}

	void Render()
	{
		if (mQuadVAO == 0)
		{
			GLfloat quadVertices[] = {
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};

			glGenVertexArrays(1, &mQuadVAO);
			glGenBuffers(1, &mQuadVBO);
			glBindVertexArray(mQuadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		}

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	
	}
};
#endif
