#pragma once
#include "GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class ExplosiveObject;
		class PlayerObject : public GameObject {
		public:
			PlayerObject() { this->name = "Player"; }
			~PlayerObject() {}
			void OnCollisionBegin(GameObject* otherObject) override;
			void OnCollisionEnd(GameObject* otherObject) override;
			void UpdateObject(float dt, GameWorld& world) override;
			void JumpInput() { jumpPressed = 0.25f; }
			void Jump();
			void Interact(GameWorld& world);
			int UseItem();
			bool JumpPressed() { return jumpPressed > 0; }
			bool IsOnFloor() { return floorTimer > 0; }
			void ScrollSelected(int i) { selectedItem = std::min(1, std::max(0, selectedItem + i)); }
			int GetSelected() { return selectedItem; }
			int GetBombCount() { return bombCount; }
			int GetC4Count() { return c4Count; }
			void AddC4(ExplosiveObject* obj) { placedC4.push_back(obj); }
			GameObject* GetHoveredObject() { return hoveredObject; }
			int GetCollectedGoals() { return collectedGoals; }
			bool IsPlayerDead() { return isDead; }
			std::vector<Vector3> GetExplodedC4Positions() { return explodedC4Positions; }
			std::vector<Transform> GetExplodedWallPositions() { return explodedWallPositions; }
		protected:
			GameObject* hoveredObject = nullptr;
			std::vector<ExplosiveObject*> placedC4;
			std::vector<Vector3> explodedC4Positions;
			std::vector<Transform> explodedWallPositions;
			float jumpPressed = 0;
			float floorTimer = 0;
			bool onFloor = false;
			bool isDead = false;
			int collectedGoals = 0;
			int bombCount = 0;
			int c4Count = 5;
			int selectedItem = 0;
		};
	}
}