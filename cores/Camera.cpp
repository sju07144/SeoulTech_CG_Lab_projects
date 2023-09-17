#include "Camera.h"
using namespace glm;

Camera::Camera()
	: mPosition(vec3(0.0f, 0.0f, 0.0f)),
	mRight(vec3(1.0f, 0.0f, 0.0f)), mUp(vec3(0.0f, 1.0f, 0.0f)), mFront(vec3(0.0f, 0.0f, 1.0f))

{
	mView = glm::mat4(1.0f);
	mProjection = glm::mat4(1.0f);
}

vec3 Camera::GetPosition()
{
	return mPosition;
}
void Camera::SetPosition(const vec3& position)
{
	mPosition = position;
}
void Camera::SetPosition(float x, float y, float z)
{
	mPosition = vec3(x, y, z);
}

vec3 Camera::GetRight()
{
	return mRight;
}
vec3 Camera::GetUp()
{
	return mUp;
}
vec3 Camera::GetFront()
{
	return mFront;
}

mat4 Camera::GetView()
{
	return mView;
}
mat4 Camera::GetProjection()
{
	return mProjection;
}

void Camera::LookAt(
	const vec3& position,
	const vec3& target,
	const vec3& worldUp)
{
	vec3 front = glm::normalize(target - position);
	vec3 right = glm::normalize(glm::cross(front, worldUp));
	vec3 up = glm::cross(right, front);

	mPosition = position;
	mRight = right;
	mUp = up;
	mFront = front;

	UpdateViewMatrix();
}
void Camera::LookAt(
	float posX, float posY, float posZ,
	float targetX, float targetY, float targetZ,
	float worldUpX, float worldUpY, float worldUpZ)
{
	Camera::LookAt(
		vec3(posX, posY, posZ),
		vec3(targetX, targetY, targetZ),
		vec3(worldUpX, worldUpY, worldUpZ));
}

void Camera::SetLens(float fovAngleY, float aspectRatio, float zn, float zf)
{
	mFovAngleY = fovAngleY;
	mAspectRatio = aspectRatio;
	mNearZ = zn;
	mFarZ = zf;

	mProjection = glm::perspective(fovAngleY, aspectRatio, zn, zf);
}

void Camera::Strafe(float distance)
{
	mPosition = distance * mRight + mPosition;

	UpdateViewMatrix();
}
void Camera::Walk(float distance)
{
	mPosition = distance * mFront + mPosition;

	UpdateViewMatrix();
}

void Camera::Pitch(float angle)
{
	mat4 rotationMatrix = glm::rotate(angle, mRight);

	mUp = mat3(rotationMatrix) * mUp;
	mFront = mat3(rotationMatrix) * mFront;

	UpdateViewMatrix();
}
void Camera::RotateY(float angle)
{
	// OpenGL coordinate is left-handed coordinate, so put angle minus.

	mRight = glm::rotateY(mRight, -angle);
	mUp = glm::rotateY(mUp, -angle);
	mFront = glm::rotateY(mFront, -angle);

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	mFront = glm::normalize(mFront);
	mUp = glm::normalize(glm::cross(mRight, mFront));
	mRight = glm::cross(mFront, mUp);

	mView = glm::lookAt(mPosition, mPosition + mFront, mUp);
}