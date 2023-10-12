#pragma once
#include "Stdafx.h"

class Camera
{
public:
	Camera();

	glm::vec3 GetPosition();
	void SetPosition(const glm::vec3& position);
	void SetPosition(float x, float y, float z);

	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetFront();

	glm::mat4 GetView();
	glm::mat4 GetProjection();
	glm::mat4 GetOrthoProjection();

	void LookAt(
		const glm::vec3& position,
		const glm::vec3& target,
		const glm::vec3& worldUp);
	void LookAt(
		float posX, float posY, float posZ,
		float targetX, float targetY, float targetZ,
		float worldUpX, float worldUpY, float worldUpZ);

	void SetLens(
		float fovAngleY,
		float aspectRatio,
		float zn,
		float zf);

	void SetOrthoLens(
		float left, 
		float right, 
		float bottom, 
		float top, 
		float zn, 
		float zf);

	void Strafe(float distance);
	void Walk(float distance);

	void Pitch(float angle);
	void RotateY(float angle);
private:
	void UpdateViewMatrix();
private:
	glm::vec3 mPosition;

	glm::vec3 mRight;
	glm::vec3 mUp;
	glm::vec3 mFront;

	// perspective variables
	float mFovAngleY = 0.0f;
	float mAspectRatio = 0.0f;
	float mNearZ = 0.0f;
	float mFarZ = 0.0f;

	// orthogonal variables
	float mOrthoLeft = 0.0f;
	float mOrthoRight = 0.0f;
	float mOrthoBottom = 0.0f;
	float mOrthoTop = 0.0f;
	float mOrthoNearZ = 0.0f;
	float mOrthoFarZ = 0.0f;

	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::mat4 mOrthoProjection;
};