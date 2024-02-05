#pragma once
#include "Constraint.h"

namespace NCL::CSC8503 {
	class GameObject;

	class RodConstraint : public Constraint {
	public:
		RodConstraint(GameObject* a, GameObject* b, float d) {
			objectA = a;
			objectB = b;
			distance = d;
		}
		~RodConstraint() {}

		void UpdateConstraint(float dt) override;
	protected:
		GameObject* objectA;
		GameObject* objectB;
		float distance;
	};
}