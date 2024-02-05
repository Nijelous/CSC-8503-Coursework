#include "Camera.h"
#include "Window.h"
#include "Vector4.h"

using namespace NCL;

/*
Polls the camera for keyboard / mouse movement.
Should be done once per frame! Pass it the msec since
last frame (default value is for simplicities sake...)
*/
void Camera::UpdateCamera(float dt) {
	if (!activeController) {
		return;
	}

	//Update the mouse by how much
	pitch	-= activeController->GetNamedAxis("YLook");
	yaw		-= activeController->GetNamedAxis("XLook");

	//Bounds check the pitch, to be between straight up and straight down ;)
	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw <0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	float frameSpeed = 100 * dt;

	Matrix4 yawRotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	position += yawRotation * Vector3(0, 0, -activeController->GetNamedAxis("Forward")) * frameSpeed;
	position += yawRotation * Vector3(activeController->GetNamedAxis("Sidestep"), 0, 0) * frameSpeed;

	position.y += activeController->GetNamedAxis("UpDown") * frameSpeed;

}

/*
Generates a view matrix for the camera's viewpoint. This matrix can be sent
straight to the shader...it's already an 'inverse camera' matrix.
*/
Matrix4 Camera::BuildViewMatrix() const {
	//Why do a complicated matrix inversion, when we can just generate the matrix
	//using the negative values ;). The matrix multiplication order is important!
	return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Translation(-position);
};

Vector3 Camera::GetUp() const {
	if (pitch <= 45 && pitch >= -45) return { 0, 1, 0 };
	else {
		float facingUp = pitch < -45 ? 1 : -1;
		if (yaw >= 315 || yaw <= 45) return { 0, 0, -facingUp };
		else if (yaw <= 315 && yaw >= 225) return { facingUp, 0, 0 };
		else if (yaw <= 225 && yaw >= 135) return { 0, 0, facingUp };
		else return { -facingUp, 0, 0 };
	}
}

Vector3 Camera::GetRight() const {
	if (yaw >= 315 || yaw <= 45) return { 1, 0, 0 };
	else if (yaw <= 315 && yaw >= 225) return { 0, 0, 1 };
	else if (yaw <= 225 && yaw >= 135) return { -1, 0, 0 };
	else return { 0, 0, -1 };
}

Vector3 Camera::GetRight180() const {
	if (yaw >= -45 && yaw <= 45) return { 1, 0, 0 };
	else if (yaw <= -45 && yaw >= -135) return { 0, 0, 1 };
	else if (yaw <= -135 || yaw >= 135) return { -1, 0, 0 };
	else return { 0, 0, -1 };
}

Vector3 Camera::GetForward() const {
	if (yaw >= 315 || yaw <= 45) return { -1, 0, 0 };
	else if (yaw <= 315 && yaw >= 225) return { 0, 0, -1 };
	else if (yaw <= 225 && yaw >= 135) return { 1, 0, 0 };
	else return { 0, 0, 1 };
}

Vector3 Camera::GetForward180() const {
	if (yaw >= -45 && yaw <= 45) return { 0, 0, -1 };
	else if (yaw <= -45 && yaw >= -135) return { 1, 0, 0 };
	else if (yaw <= -135 || yaw >= 135) return { 0, 0, 1 };
	else return { -1, 0, 0 };
}

Vector3 Camera::GetCameraDirection() const
{
	Matrix4 view = BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0));
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();
	Vector3 downAxis = Vector3::Cross(fwdAxis, rightAxis);
	return (fwdAxis + (-downAxis * (pitch / 90))).Normalised();
}

Matrix4 PerspectiveCamera::BuildProjectionMatrix(float currentAspect) const {
	return Matrix4::Perspective(nearPlane, farPlane, currentAspect, fov);
}

Matrix4 OrhographicCamera::BuildProjectionMatrix(float currentAspect) const {
	return Matrix4::Orthographic(left, right, bottom, top, nearPlane, farPlane);
}