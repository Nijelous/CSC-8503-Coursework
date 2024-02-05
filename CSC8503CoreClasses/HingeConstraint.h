#pragma once
#include "Constraint.h"

namespace NCL::CSC8503 {
	class GameObject;

	class HingeConstraint : public Constraint {
	public:
		HingeConstraint(GameObject* door, GameObject* hinge) {
			this->door = door;
			this->hinge = hinge;
		}
		~HingeConstraint() {}

		void UpdateConstraint(float dt) override;
	protected:
		GameObject* door;
		GameObject* hinge;
	};
}