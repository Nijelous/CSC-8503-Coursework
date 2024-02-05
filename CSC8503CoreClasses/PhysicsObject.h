#pragma once
using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);

			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			void InitCubeInertia();
			void InitSphereInertia();

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}
			bool IsSleeping() { return sleeping; }
			void Sleep(bool gravityOn, Vector3 gravity, Vector3 pos) {
				sleepCount++; 
				if (sleepCount > 25 && linearVelocity < Vector3(0.01f, gravity.y * 0.01f, 0.01f)) {
					sleeping = true;
					linearVelocity = { 0, 0, 0 };
					angularVelocity = { 0, 0, 0 };
					ClearForces();
					sleepPosition = pos;
				}
			}
			void Awaken() { sleepCount = 0; sleeping = false; }

			
			bool IsColliding() { return colliding; }
			void SetColliding(bool c) { colliding = c; }

			Vector3 GetSleepPos() { return sleepPosition; }

		protected:
			const CollisionVolume* volume;
			Transform*		transform;

			float inverseMass;
			float elasticity;
			float friction;
			int sleepCount;
			bool sleeping;
			bool colliding;
			Vector3 sleepPosition;

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;
			
			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
		};
	}
}

