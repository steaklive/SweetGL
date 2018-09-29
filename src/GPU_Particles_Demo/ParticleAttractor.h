#pragma once


#include "vmath.h"


class ParticleAttractor 
{
private:
	vmath::vec4 mPosition;
	float mDepth;
	bool mIsActive;

public:
	ParticleAttractor() : mPosition(), mDepth(0.0f), mIsActive(false)
	{
	}

	void updateAttractor(const Camera<UpdatePolicy>&camera, const GLFWInput& input) {
		// Update attractor depth
		if (input.isKeyPressed(GLFW_KEY_E)) {
			incrementDepth(1.0f);
		}
		if (input.isKeyPressed(GLFW_KEY_Q)) {
			incrementDepth(-1.0f);
		}

		// When right mouse button is not pressed the attractor is not used.
		if (!input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
			_active = false;
			return;
		}

		if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			_active = true;
		}
		else {
			_active = false;
		}

		glm::vec3 view, h, v, pos, dir, attractor(0);
		auto width = static_cast<float>(camera.getWidth());
		auto height = static_cast<float>(camera.getHeight());
		float aspectRatio = width / height;

		//Compute the coordinate axes
		view = glm::vec3(camera.getLookToVector());
		h = glm::vec3(camera.getRightVector());
		v = glm::vec3(camera.getUpVector());

		//Scale them
		float vLength = glm::tan(camera.getFOV() * PI / 360.f) * camera.getNearPlane();

		v *= vLength;
		h *= vLength * aspectRatio;


		float mouseX = (static_cast<float>(input.getXPos()) - (width / 2.0f)) / (width / 2.0f);	//Map the coordinate to [-1, 1]
		float mouseY = (static_cast<float>(input.getYPos()) - (height / 2.0f)) / (height / 2.0f); //Map the coordinate to [-1, 1]

		//Compute the intersection with the near plane
		pos = glm::vec3(camera.getPosition()) + view * camera.getNearPlane() + h * mouseX - v * mouseY;
		//Compute the direction of the ray
		dir = glm::normalize(pos - glm::vec3(camera.getPosition()));

		//Shoot attractor along the ray to the given depth
		attractor = pos + dir * _attDepth;

		//Update attractor
		_position = glm::vec4(attractor, 1.0f);
	}

	void ChangeDepth(float inc)
	{
		mDepth += inc;
		if (mDepth < 0) {
			mDepth = 0;
		}
		else if (mDepth > 1000) {
			mDepth = 1000;
		}
	}

	vmath::vec4 GetPosition() const
	{
		return mPosition;
	}

	float GetDepth() const
	{
		return mDepth;
	}

	bool IsActive() const 
	{
		return mIsActive;
	}
};
