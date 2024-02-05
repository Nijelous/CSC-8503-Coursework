#pragma once
#include "GameObject.h"
#include "GameWorld.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "NavigationGrid.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		enum class ObjectType {
			Floor,
			Wall,
			Cube,
			OrientedCube,
			Sphere,
			Capsule,
			Player,
			Bomb,
			Goal,
			Home,
			Ramp,
			Door,
			Button,
			Enemy,
			Invalid
		};
		struct LevelItem {
			ObjectType type;
			Vector3 position;
			Quaternion orientation;
			Vector3 scale;
			LevelItem(ObjectType type, Vector3 position, Quaternion orientation, Vector3 scale) {
				this->type = type;
				this->position = position;
				this->orientation = orientation;
				this->scale = scale;
			}
			LevelItem() { type = ObjectType::Invalid; }
		};

		class Level
		{
		public:
			void WriteLevel(GameWorld& world, std::string name);
			void ClearLevelFileRaw() { levelFileRaw.clear(); }
			bool WriteItemNetwork(std::string item, std::string name, int expectedItems);
			void WriteNavMeshNetwork(std::string item, std::string name, int index, int expectedItems);
			bool LoadNavMeshNetwork(std::string item, int index, int expectedItems);
			void LoadLevel(GameWorld& world, std::string name);
			void LoadItemFromServer(std::string item);
			bool LoadFullLevel(GameWorld& world, int expectedItems);
			std::vector<std::string> GetLevelObjectFile(std::string name);
			std::vector<std::string> GetLevelNavMesh(GameWorld& world);
			std::vector<std::string> GetLevelNavMeshFile(std::string name);
			std::vector<std::string> GetLevelObjectWorld(GameWorld& world);
			void RecordHighScore(std::string name, int highScore);
			std::vector<int> GetHighScores(std::string name);
			void LoadRenderStuff(Mesh* capsuleMesh, Mesh* cubeMesh, Mesh* sphereMesh, Mesh* playerMesh, Mesh* enemyMesh, Texture* basicTex, Shader* basicShader) {
				this->capsuleMesh = capsuleMesh;
				this->cubeMesh = cubeMesh;
				this->sphereMesh = sphereMesh;
				this->playerMesh = playerMesh;
				this->enemyMesh = enemyMesh;
				this->basicTex = basicTex;
				this->basicShader = basicShader;
			}
			Vector4 GetButtonColour(int id);
			NavigationGrid GetGridOffline() { return NavigationGrid(gridPath); }
			NavigationGrid GetGridOnline() { return NavigationGrid(navGridRaw); }
			void ClearObjects() { objects.clear(); }
		protected:
			void AddObject(const GameObject* object, Transform& transform);
			void CreateFloor(LevelItem object, GameWorld& world);
			void CreateWall(LevelItem object, GameWorld& world);
			void CreateCube(LevelItem object, GameWorld& world);
			void CreateOrientedCube(LevelItem object, GameWorld& world);
			void CreateSphere(LevelItem object, GameWorld& world);
			void CreateCapsule(LevelItem object, GameWorld& world);
			void CreatePlayer(LevelItem object, GameWorld& world);
			void CreateBomb(LevelItem object, GameWorld& world);
			void CreateGoal(LevelItem object, GameWorld& world);
			void CreateHome(LevelItem object, GameWorld& world);
			void CreateRamp(LevelItem object, GameWorld& world);
			void CreateDoor(LevelItem object, GameWorld& world);
			void CreateButton(LevelItem object, GameWorld& world);
			void CreateEnemy(LevelItem object, GameWorld& world);
			std::vector<LevelItem> objects;
			std::vector<int> highScores;
			std::vector<std::string> levelFileRaw;
			std::vector<std::string> navGridRaw;
			LevelItem mainFloor{};
			std::map<int, std::string> navGridMap;
			int itemsRecieved = 0;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;
			Mesh* playerMesh = nullptr;
			Mesh* enemyMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			int validLevel = 0;

			std::string gridPath;
		};
	}
}


