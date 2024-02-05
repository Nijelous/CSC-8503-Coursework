#include "HingeConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"

using namespace NCL::CSC8503;

void HingeConstraint::UpdateConstraint(float dt) {
	Vector3 relativePos = door->GetTransform().GetPosition() - hinge->GetTransform().GetPosition();
	Vector3 relativeDir = relativePos.Normalised();

	float newAngle = RadiansToDegrees(atan2(-relativeDir.z, relativeDir.x));

	hinge->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, newAngle, 0));
	door->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, newAngle+90, 0));
}
