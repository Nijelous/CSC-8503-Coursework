#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "PushdownMachine.h"
#include "Level.h"
#include "NetworkBase.h"

#include "StateGameObject.h"
#include <GameClient.h>
#include <GameServer.h>


namespace NCL {
	namespace CSC8503 {
		enum class KeyType {
			Tutorial = 1,
			LevelEditor = 2,
			Player = 4,
			Invalid = 256
		};

		enum class EditorTool {
			Move = 1,
			Rotate = 2,
			Scale = 4,
			AddObject = 8,
			ChangeMass = 16,
			Max = 32
		};

		class TutorialGame : public PacketReceiver		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual bool UpdateGame(float dt);

			Mesh* GetCubeMesh() { return cubeMesh; }
			Mesh* GetSphereMesh() { return sphereMesh; }
			Texture* GetBasicTexture() { return basicTex; }
			Shader* GetBasicShader() { return basicShader; }

			void ReceivePacket(int type, GamePacket* payload, int source) override;

		protected:
			void InitialiseAssets();
			void InitialiseClient();
			void InitialiseServer();

			void InitCamera();
			void UpdateKeys();
			void UpdateTutorialKeys();
			void UpdateLevelEditorKeys();
			void UpdatePlayerKeys();
			void MoveLevelEditor(const Vector3& moveDirection, const Vector3& rotateDirection);
			void ScaleObject(const Vector3& scaleValue);
			void ScaleAABB(AABBVolume& volume, const Vector3& scaleValue);
			void ScaleOBB(OBBVolume& volume, const Vector3& scaleValue);
			void ScaleSphere(SphereVolume& volume, float scaleValue);
			void ScaleCapsule(CapsuleVolume& volume, const Vector3& scaleValue);
			std::string GetAddName(int i);

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddToWorld(int i);
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddOrientedCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddBombToWorld(const Vector3& position);
			GameObject* AddGoalToWorld(const Vector3& position);
			GameObject* AddHomeToWorld(const Vector3& position);
			GameObject* AddDoorToWorld(const Vector3& position);
			GameObject* AddButtonToWorld(const Vector3& position);
			GameObject* ThrowBomb(const Vector3& position, const Vector3& direction, float timeToBlow);
			GameObject* PlaceC4(const Vector3& position, const Vector3& directionFromWall, GameObject* wall);
			void SplitWall(Transform wall);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);

			void BridgeConstraintTest();

			StateGameObject* testStateObject;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;
			Level level;
			vector<Vector3> testNodes;
			PushdownMachine* machine;

			bool useGravity;
			bool inSelectionMode;
			bool smoothObjectMovement = false;
			bool showBoundingVolume = false;
			bool writingLevel = false;
			bool loadingLevel = false;
			bool editing = false;
			bool networkingSetUp = false;
			bool offline = false;
			bool isClient = false;
			int expectedItems = INT_MAX;
			bool levelLoaded = false;
			GameClient* client = nullptr;
			GameServer* server = nullptr;
			std::string levelName = "";
			std::string clientLevelNames[4];
			int count = 0;
			vector<std::string> levels;
			vector<int> highScores;
			float maxTime;
			float timeLeft;

			float		forceMagnitude;
			float		movementMagnitude;
			float		rotationMagnitude;
			float		scaleMagnitude;

			GameObject* selectionObject = nullptr;
			Vector4 selectionObjectColour;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			Quaternion lockedPitch = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
			
			KeyType keyType = KeyType::Tutorial;
			EditorTool editorTool = EditorTool::Move;
		};
	}
}

