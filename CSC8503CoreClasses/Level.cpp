#include "C:/Users/c0009141/Documents/CSC8503 2023/CSC8503CoreClasses/CMakeFiles/CSC8503CoreClasses.dir/Debug/cmake_pch.hxx"
#include "Level.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "HomeObject.h"
#include "DoorObject.h"
#include "ButtonObject.h"
#include "EnemyObject.h"
#include "HingeConstraint.h"
#include "PositionConstraint.h"
#include "../CSC8503/PlayerObject.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace NCL;
using namespace CSC8503;

void Level::WriteLevel(GameWorld& world, std::string name) {
	validLevel = 0;
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world.GetObjectIterators(first, last);
	objects.clear();
	for (auto i = first; i != last; i++) {
		(*i)->GetTransform();
		AddObject(*i, (*i)->GetTransform());
		if (objects.back().type == ObjectType::Floor) {
			if (mainFloor.type == ObjectType::Invalid) mainFloor = objects.back();
			else if (mainFloor.position.y > objects.back().position.y) mainFloor = objects.back();
		}
	}

	if (mainFloor.type == ObjectType::Invalid) {
		std::cout << "This level needs a floor!\n";
		return;
	}
	if (validLevel != 7) {
		if (!(validLevel & 1)) std::cout << "This level needs a player!\n";
		if (!(validLevel & (1 << 1))) std::cout << "This level needs a goal!\n";
		if (!(validLevel & (1 << 2))) std::cout << "This level needs a home!\n";
		return;
	}

	std::filesystem::path directoryPath = "Levels/" + name;
	if (!std::filesystem::exists(directoryPath)) {
		std::filesystem::create_directory(directoryPath);
	}
	std::ofstream objectFile(directoryPath / "objects.txt");

	for (int i = 0; i < objects.size(); i++) {
		objectFile << (int)objects[i].type << ","
			<< objects[i].position.x << "," << objects[i].position.y << "," << objects[i].position.z << ","
			<< objects[i].orientation.x << "," << objects[i].orientation.y << "," << objects[i].orientation.z << "," << objects[i].orientation.w << ","
			<< objects[i].scale.x << "," << objects[i].scale.y << "," << objects[i].scale.z << "\n";
	}
	objectFile.close();

	std::string navGridFull = "";
	int minConsecutive = INT_MAX;
	int countX = 1;
	std::map<int, int> countZ;
	for (int z = 0; z < mainFloor.scale.z; z++) {
		countX = 1;
		for (int x = 0; x < mainFloor.scale.x; x++) {
			Vector3 rayPos = Vector3(x - (mainFloor.scale.x / 2), mainFloor.position.y + (mainFloor.scale.y / 2) + 0.1f, z - (mainFloor.scale.z / 2));
			Ray ray(rayPos, Vector3(1, 1, 1).Normalised());

			RayCollision closestCollision;
			if (world.Raycast(ray, closestCollision, true)) {
				if (closestCollision.rayDistance > 1) {
					navGridFull.push_back('.');
					continue;
				}
				GameObject* obj = (GameObject*)closestCollision.node;
				Vector3 scale = obj->GetTransform().GetScale();
				if (!(obj->GetName() == "Hinge") && ((obj->GetName() == "Wall") || (obj->GetBoundingVolume()->type == VolumeType::AABB
					&& obj->GetPhysicsObject()->GetInverseMass() == 0 && (scale.y >= scale.x || scale.y >= scale.z)))) {
					navGridFull.push_back('x');
				}
				else navGridFull.push_back('.');
			}
			else navGridFull.push_back('.');
			if (x > 0) {
				int index = (mainFloor.scale.x * z) + x;
				int index2 = (mainFloor.scale.x * z) + x - 1;
				if (navGridFull[(mainFloor.scale.x * z) + x] == navGridFull[(mainFloor.scale.x * z) + x - 1]) {
					countX++;
			}
				else {
					minConsecutive = std::min(countX, minConsecutive);
					countX = 1;
				}
			}
			if (z > 0) {
				if (navGridFull[(mainFloor.scale.x * z) + x] == navGridFull[(mainFloor.scale.x * (z - 1)) + x]) {
					countZ[x] += 1;
				}
				else {
					minConsecutive = std::min(countZ[x], minConsecutive);
					countZ[x] = 1;
				}
			}
			else countZ[x] = 1;
		}
		if(countX > 1) minConsecutive = std::min(countX, minConsecutive);
	}
	for (int i = 0; i < mainFloor.scale.x; i++) {
		if(countZ[i] > 1) minConsecutive = std::min(countZ[i], minConsecutive);
	}

	std::cout << minConsecutive << "\n";

	std::ofstream navGridFile(directoryPath / "navGrid.txt");

	navGridFile << minConsecutive << "\n" <<
		(int)(mainFloor.scale.x / minConsecutive)+1 << "\n" <<
		(int)(mainFloor.scale.z / minConsecutive)+1 << "\n" <<
		mainFloor.position.z - (mainFloor.scale.z / 2) << "\n" <<
		mainFloor.position.x - (mainFloor.scale.x / 2) << "\n";


	for (int i = 0; i < mainFloor.scale.z; i += minConsecutive) {
		for (int j = 0; j < mainFloor.scale.x; j += minConsecutive) {
			bool writeWall = false;
			if (j > 0 && j < mainFloor.scale.x - minConsecutive) {
				if (navGridFull[(i * mainFloor.scale.z) + j - minConsecutive] == 'x' || (navGridFull[(i * mainFloor.scale.z) + j + minConsecutive] == 'x')) {
					writeWall = true;
				}
			}
			if (i > 0 && i < mainFloor.scale.z - minConsecutive && !writeWall) {
				if (navGridFull[((i + minConsecutive) * mainFloor.scale.z) + j] == 'x' || (navGridFull[((i - minConsecutive) * mainFloor.scale.z) + j] == 'x')) {
					writeWall = true;
				}
			}

			if (writeWall) navGridFile << 'x';
			else navGridFile << navGridFull[(i * mainFloor.scale.z) + j];
		}
		navGridFile << "\n";
	}

	navGridFile.close();

	std::cout << "Level file written!\n";
}

bool Level::WriteItemNetwork(std::string item, std::string name, int expectedItems) {
	levelFileRaw.push_back(item);
	if (expectedItems == levelFileRaw.size()) {
		std::filesystem::path directoryPath = "Levels/" + name;
		if (!std::filesystem::exists(directoryPath)) {
			std::filesystem::create_directory(directoryPath);
		}
		std::ofstream objectFile(directoryPath / "objects.txt");

		for (int i = 0; i < levelFileRaw.size(); i++) {
			objectFile << levelFileRaw[i] << "\n";
		}
		objectFile.close();
		std::cout << "Level file written!\n";
		return true;
	}
	return false;
}

void Level::WriteNavMeshNetwork(std::string item, std::string name, int index, int expectedItems) {
	navGridMap[index] = item;
	itemsRecieved++;
	if (itemsRecieved == expectedItems) {
		std::filesystem::path directoryPath = "Levels/" + name;
		if (!std::filesystem::exists(directoryPath)) {
			std::filesystem::create_directory(directoryPath);
		}
		std::ofstream navGridFile(directoryPath / "navGrid.txt");
		for (int i = 0; i < expectedItems; i++) {
			navGridFile << navGridMap[i] << "\n";
		}
		navGridFile.close();
		std::cout << "Nav Grid file written!\n";
		itemsRecieved = 0;
	}
}

bool Level::LoadNavMeshNetwork(std::string item, int index, int expectedItems) {
	navGridMap[index] = item;
	itemsRecieved++;
	if (itemsRecieved == expectedItems) {
		navGridRaw.clear();
		for (int i = 0; i < expectedItems; i++) {
			navGridRaw.push_back(navGridMap[i]);
		}
		itemsRecieved = 0;
		return true;
	}
	return false;
}

void Level::LoadLevel(GameWorld& world, std::string name)
{
	std::filesystem::path directoryPath = name;
	std::ifstream objectFile(directoryPath / "objects.txt");
	std::string line;
	objects.clear();
	while (getline(objectFile, line)) {
		std::string currentNumber = "";
		float values[11];
		int count = 0;
		for (char c : line) {
			if (c == ',') {
				values[count] = std::stof(currentNumber);
				count++;
				currentNumber = "";
			}
			else {
				currentNumber.push_back(c);
			}
		}
		values[count] = std::stof(currentNumber);
		objects.push_back(LevelItem((ObjectType)values[0],
			Vector3(values[1], values[2], values[3]),
			Quaternion(values[4], values[5], values[6], values[7]),
			Vector3(values[8], values[9], values[10])));
	}
	for (int i = 0; i < objects.size(); i++) {
		switch (objects[i].type) {
		case ObjectType::Floor:
			CreateFloor(objects[i], world);
			break;
		case ObjectType::Wall:
			CreateWall(objects[i], world);
			break;
		case ObjectType::Cube:
			CreateCube(objects[i], world);
			break;
		case ObjectType::OrientedCube:
			CreateOrientedCube(objects[i], world);
			break;
		case ObjectType::Sphere:
			CreateSphere(objects[i], world);
			break;
		case ObjectType::Capsule:
			CreateCapsule(objects[i], world);
			break;
		case ObjectType::Player:
			CreatePlayer(objects[i], world);
			break;
		case ObjectType::Bomb:
			CreateBomb(objects[i], world);
			break;
		case ObjectType::Goal:
			CreateGoal(objects[i], world);
			break;
		case ObjectType::Home:
			CreateHome(objects[i], world);
			break;
		case ObjectType::Ramp:
			CreateRamp(objects[i], world);
			break;
		case ObjectType::Door:
			CreateDoor(objects[i], world);
			break;
		case ObjectType::Button:
			CreateButton(objects[i], world);
			break;
		case ObjectType::Enemy:
			CreateEnemy(objects[i], world);
			break;
		}
	}
	objectFile.close();
	gridPath = name + "/navGrid.txt";
}

void Level::LoadItemFromServer(std::string item) {
	std::string currentNumber = "";
	float values[11];
	int count = 0;
	for (char c : item) {
		if (c == ',') {
			values[count] = std::stof(currentNumber);
			count++;
			currentNumber = "";
		}
		else {
			currentNumber.push_back(c);
		}
	}
	values[count] = std::stof(currentNumber);
	objects.push_back(LevelItem((ObjectType)values[0],
		Vector3(values[1], values[2], values[3]),
		Quaternion(values[4], values[5], values[6], values[7]),
		Vector3(values[8], values[9], values[10])));
}

bool Level::LoadFullLevel(GameWorld& world, int expectedItems) {
	if (expectedItems == objects.size()) {
		for (int i = 0; i < objects.size(); i++) {
			switch (objects[i].type) {
			case ObjectType::Floor:
				CreateFloor(objects[i], world);
				break;
			case ObjectType::Wall:
				CreateWall(objects[i], world);
				break;
			case ObjectType::Cube:
				CreateCube(objects[i], world);
				break;
			case ObjectType::OrientedCube:
				CreateOrientedCube(objects[i], world);
				break;
			case ObjectType::Sphere:
				CreateSphere(objects[i], world);
				break;
			case ObjectType::Capsule:
				CreateCapsule(objects[i], world);
				break;
			case ObjectType::Player:
				CreatePlayer(objects[i], world);
				break;
			case ObjectType::Bomb:
				CreateBomb(objects[i], world);
				break;
			case ObjectType::Goal:
				CreateGoal(objects[i], world);
				break;
			case ObjectType::Home:
				CreateHome(objects[i], world);
				break;
			case ObjectType::Ramp:
				CreateRamp(objects[i], world);
				break;
			case ObjectType::Door:
				CreateDoor(objects[i], world);
				break;
			case ObjectType::Button:
				CreateButton(objects[i], world);
				break;
			case ObjectType::Enemy:
				CreateEnemy(objects[i], world);
				break;
			}
		}
		return true;
	}
	return false;
}

std::vector<std::string> Level::GetLevelObjectFile(std::string name) {
	std::filesystem::path directoryPath = "Levels/" + name;
	std::ifstream objectFile(directoryPath / "objects.txt");
	std::string line;
	levelFileRaw.clear();
	while (getline(objectFile, line)) {
		levelFileRaw.push_back(line);
	}
	return levelFileRaw;
}

std::vector<std::string> Level::GetLevelNavMesh(GameWorld& world) {
	std::string navGridFull = "";
	navGridRaw.clear();
	int minConsecutive = INT_MAX;
	int countX = 1;
	std::map<int, int> countZ;
	for (int z = 0; z < mainFloor.scale.z; z++) {
		countX = 1;
		for (int x = 0; x < mainFloor.scale.x; x++) {
			Vector3 rayPos = Vector3(x - (mainFloor.scale.x / 2), mainFloor.position.y + (mainFloor.scale.y / 2) + 0.1f, z - (mainFloor.scale.z / 2));
			Ray ray(rayPos, Vector3(1, 1, 1).Normalised());

			RayCollision closestCollision;
			if (world.Raycast(ray, closestCollision, true)) {
				if (closestCollision.rayDistance > 1) {
					navGridFull.push_back('.');
					continue;
				}
				GameObject* obj = (GameObject*)closestCollision.node;
				Vector3 scale = obj->GetTransform().GetScale();
				if (!(obj->GetName() == "Hinge") && ((obj->GetName() == "Wall") || (obj->GetBoundingVolume()->type == VolumeType::AABB
					&& obj->GetPhysicsObject()->GetInverseMass() == 0 && (scale.y >= scale.x || scale.y >= scale.z)))) {
					navGridFull.push_back('x');
				}
				else navGridFull.push_back('.');
			}
			else navGridFull.push_back('.');
			if (x > 0) {
				int index = (mainFloor.scale.x * z) + x;
				int index2 = (mainFloor.scale.x * z) + x - 1;
				if (navGridFull[(mainFloor.scale.x * z) + x] == navGridFull[(mainFloor.scale.x * z) + x - 1]) {
					countX++;
				}
				else {
					minConsecutive = std::min(countX, minConsecutive);
					countX = 1;
				}
			}
			if (z > 0) {
				if (navGridFull[(mainFloor.scale.x * z) + x] == navGridFull[(mainFloor.scale.x * (z - 1)) + x]) {
					countZ[x] += 1;
				}
				else {
					minConsecutive = std::min(countZ[x], minConsecutive);
					countZ[x] = 1;
				}
			}
			else countZ[x] = 1;
		}
		if (countX > 1) minConsecutive = std::min(countX, minConsecutive);
	}
	for (int i = 0; i < mainFloor.scale.x; i++) {
		if (countZ[i] > 1) minConsecutive = std::min(countZ[i], minConsecutive);
	}

	std::cout << minConsecutive << "\n";

	if (mainFloor.scale.z / minConsecutive > 245) {
		std::cout << "Can't save on server, make your mesh more efficient!\n";
		return navGridRaw;
	}

	navGridRaw.push_back(std::to_string(minConsecutive));
	navGridRaw.push_back(std::to_string((int)(mainFloor.scale.x / minConsecutive) + 1));
	navGridRaw.push_back(std::to_string((int)(mainFloor.scale.z / minConsecutive) + 1));
	navGridRaw.push_back(std::to_string(mainFloor.position.z - (mainFloor.scale.z / 2)));
	navGridRaw.push_back(std::to_string(mainFloor.position.x - (mainFloor.scale.x / 2)));

	std::string temp = "";

	for (int i = 0; i < mainFloor.scale.z; i += minConsecutive) {
		for (int j = 0; j < mainFloor.scale.x; j += minConsecutive) {
			bool writeWall = false;
			if (j > 0 && j < mainFloor.scale.x - minConsecutive) {
				if (navGridFull[(i * mainFloor.scale.z) + j - minConsecutive] == 'x' || (navGridFull[(i * mainFloor.scale.z) + j + minConsecutive] == 'x')) {
					writeWall = true;
				}
			}
			if (i > 0 && i < mainFloor.scale.z - minConsecutive && !writeWall) {
				if (navGridFull[((i + minConsecutive) * mainFloor.scale.z) + j] == 'x' || (navGridFull[((i - minConsecutive) * mainFloor.scale.z) + j] == 'x')) {
					writeWall = true;
				}
			}

			if (writeWall) temp.push_back('x');
			else temp.push_back(navGridFull[(i * mainFloor.scale.z) + j]);
		}
		navGridRaw.push_back(temp);
		temp = "";
	}
	return navGridRaw;
}

std::vector<std::string> Level::GetLevelNavMeshFile(std::string name)
{
	std::filesystem::path directoryPath = "Levels/" + name;
	std::ifstream objectFile(directoryPath / "navGrid.txt");
	std::string line;
	navGridRaw.clear();
	while (getline(objectFile, line)) {
		navGridRaw.push_back(line);
	}
	return navGridRaw;
}

std::vector<std::string> Level::GetLevelObjectWorld(GameWorld& world) {
	validLevel = 0;
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world.GetObjectIterators(first, last);
	objects.clear();
	levelFileRaw.clear();
	for (auto i = first; i != last; i++) {
		(*i)->GetTransform();
		AddObject(*i, (*i)->GetTransform());
		if (objects.back().type == ObjectType::Floor) {
			if (mainFloor.type == ObjectType::Invalid) mainFloor = objects.back();
			else if (mainFloor.position.y > objects.back().position.y) mainFloor = objects.back();
		}
	}

	if (mainFloor.type == ObjectType::Invalid) {
		std::cout << "This level needs a floor!\n";
		return levelFileRaw;
	}
	if (validLevel != 7) {
		if (!(validLevel & 1)) std::cout << "This level needs a player!\n";
		if (!(validLevel & (1 << 1))) std::cout << "This level needs a goal!\n";
		if (!(validLevel & (1 << 2))) std::cout << "This level needs a home!\n";
		return levelFileRaw;
	}

	for (int i = 0; i < objects.size(); i++) {
		levelFileRaw.push_back(std::format("{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10}", 
			(int)objects[i].type, objects[i].position.x, objects[i].position.y, objects[i].position.z, 
			objects[i].orientation.x, objects[i].orientation.y, objects[i].orientation.z, objects[i].orientation.w,
			objects[i].scale.x, objects[i].scale.y, objects[i].scale.z));
	}
	return levelFileRaw;
}

void Level::RecordHighScore(std::string name, int highScore) {
	std::filesystem::path directoryPath = "Levels/" + name;
	std::ofstream objectFile(directoryPath / "highScores.txt", std::ios_base::app);

	objectFile << highScore << "\n";

	objectFile.close();
}

std::vector<int> Level::GetHighScores(std::string name) {
	std::filesystem::path directoryPath = "Levels/" + name;
	std::ifstream objectFile(directoryPath / "highScores.txt");
	highScores.clear();
	std::string line;
	while (getline(objectFile, line)) {
		if (line == "") break;
		highScores.push_back(std::stoi(line));
	}
	return highScores;
}

void Level::AddObject(const GameObject* object, Transform& transform)
{
	Vector3 position = transform.GetPosition();
	Quaternion orientation = transform.GetOrientation();
	Vector3 scale = transform.GetScale();
	ObjectType type = ObjectType::Invalid;
	if (object->GetBoundingVolume()->type == VolumeType::Sphere) {
		if (object->GetName() == "Bomb") type = ObjectType::Bomb;
		else if (object->GetName() == "Goal") {
			validLevel = validLevel | (1 << 1);
			type = ObjectType::Goal;
		}
		else if (object->GetName() == "Enemy") type = ObjectType::Enemy;
		else type = ObjectType::Sphere;
	}
	else if (object->GetBoundingVolume()->type == VolumeType::AABB) {
		if (object->GetPhysicsObject()->GetInverseMass() == 0) {
			if (scale.y < scale.x && scale.y < scale.z) {
				if (object->GetName() == "Home") {
					validLevel = validLevel | (1 << 2);
					type = ObjectType::Home;
				}
				else type = ObjectType::Floor;
			}
			else if (object->GetName() == "C4" || object->GetName() == "Hinge") type = ObjectType::Invalid;
			else type = ObjectType::Wall;
		}
		else type = ObjectType::Cube;
	}
	else if (object->GetBoundingVolume()->type == VolumeType::OBB) {
		if (object->GetPhysicsObject()->GetInverseMass() == 0) {
			if (object->GetName() == "Door") type = ObjectType::Door;
			else if (object->GetName() == "Button") type = ObjectType::Button;
			else type = ObjectType::Ramp;
			
		}
		else type = ObjectType::OrientedCube;
	}
	else if (object->GetBoundingVolume()->type == VolumeType::Capsule) {
		if (object->GetName() == "Player") {
			validLevel = validLevel | 1;
			type = ObjectType::Player;
		}
		else type = ObjectType::Capsule;
	}
	if (type != ObjectType::Invalid) {
		if (type == ObjectType::Button) {
			ButtonObject* obj = (ButtonObject*)object;
			objects.push_back(LevelItem(type, position, orientation, Vector3(obj->GetButtonID(), scale.y, scale.z)));
		}
		else if (type == ObjectType::Door) {
			DoorObject* obj = (DoorObject*)object;
			if (obj->GetHinge()) objects.push_back(LevelItem(type, position, orientation * Quaternion(0.0f, 1.0f, 0.0f, 0.0f), Vector3(obj->GetButton(), scale.y, scale.z)));
			else objects.push_back(LevelItem(type, position, orientation, Vector3(obj->GetButton(), scale.y, scale.z)));
		}
		else objects.push_back(LevelItem(type, position, orientation, scale));
	}
}

void Level::CreateFloor(LevelItem object, GameWorld& world) {
	GameObject* floor = new GameObject("Floor");

	AABBVolume* volume = new AABBVolume(object.scale/2);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(floor);
}

void Level::CreateWall(LevelItem object, GameWorld& world) {
	GameObject* wall = new GameObject("Wall");

	AABBVolume* volume = new AABBVolume(object.scale / 2);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(wall);
}

void Level::CreateCube(LevelItem object, GameWorld& world) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(object.scale / 2);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(10.0f);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);
}

void Level::CreateOrientedCube(LevelItem object, GameWorld& world)
{
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(object.scale/2);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(10.0f);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);
}

void Level::CreateSphere(LevelItem object, GameWorld& world) {
	GameObject* sphere = new GameObject();

	SphereVolume* volume = new SphereVolume(object.scale.x);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(10.0f);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(sphere);
}

void Level::CreateCapsule(LevelItem object, GameWorld& world) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(0.5f * object.scale.y, 0.5f * object.scale.x);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(5.0f);
	capsule->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(capsule);
}

void Level::CreatePlayer(LevelItem object, GameWorld& world) {
	PlayerObject* player = new PlayerObject();

	CapsuleVolume* volume = new CapsuleVolume(0.5f * object.scale.y, 0.5f * object.scale.x);
	player->SetBoundingVolume((CollisionVolume*)volume);

	player->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), playerMesh, nullptr, basicShader));
	player->GetRenderObject()->SetColour(Vector4(1, 0.5f, 0, 1));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(5.0f);
	player->GetPhysicsObject()->InitSphereInertia();

	world.AddPlayer(player);
}

void Level::CreateBomb(LevelItem object, GameWorld& world) {
	GameObject* bomb = new GameObject("Bomb");

	SphereVolume* volume = new SphereVolume(object.scale.x);
	bomb->SetBoundingVolume((CollisionVolume*)volume);

	bomb->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	bomb->SetRenderObject(new RenderObject(&bomb->GetTransform(), sphereMesh, basicTex, basicShader));
	bomb->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	bomb->SetPhysicsObject(new PhysicsObject(&bomb->GetTransform(), bomb->GetBoundingVolume()));

	bomb->GetPhysicsObject()->SetInverseMass(0.0f);
	bomb->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(bomb);
}

void Level::CreateGoal(LevelItem object, GameWorld& world) {
	GameObject* goal = new GameObject("Goal");

	SphereVolume* volume = new SphereVolume(object.scale.x);
	goal->SetBoundingVolume((CollisionVolume*)volume);

	goal->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	goal->SetRenderObject(new RenderObject(&goal->GetTransform(), sphereMesh, nullptr, basicShader));
	goal->GetRenderObject()->SetColour(Vector4(0.65f, 0.486f, 0, 1));
	goal->SetPhysicsObject(new PhysicsObject(&goal->GetTransform(), goal->GetBoundingVolume()));

	goal->GetPhysicsObject()->SetInverseMass(0.0f);
	goal->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(goal);
}

void Level::CreateHome(LevelItem object, GameWorld& world) {
	HomeObject* home = new HomeObject();

	AABBVolume* volume = new AABBVolume(object.scale / 2);
	home->SetBoundingVolume((CollisionVolume*)volume);
	home->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	home->SetRenderObject(new RenderObject(&home->GetTransform(), cubeMesh, nullptr, basicShader));
	home->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	home->SetPhysicsObject(new PhysicsObject(&home->GetTransform(), home->GetBoundingVolume()));

	home->GetPhysicsObject()->SetInverseMass(0.0f);
	home->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(home);
}

void Level::CreateRamp(LevelItem object, GameWorld& world) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(object.scale / 2);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0.0f);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);
}

void Level::CreateDoor(LevelItem object, GameWorld& world) {
	DoorObject* door = new DoorObject(object.scale.x);

	Vector3 dimensions = Vector3(1, object.scale.y, object.scale.z);
	OBBVolume* volume = new OBBVolume(dimensions / 2);
	door->SetBoundingVolume((CollisionVolume*)volume);
	door->GetTransform()
		.SetScale(dimensions)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	door->SetRenderObject(new RenderObject(&door->GetTransform(), cubeMesh, basicTex, basicShader));
	door->GetRenderObject()->SetColour(GetButtonColour(object.scale.x));
	door->SetPhysicsObject(new PhysicsObject(&door->GetTransform(), door->GetBoundingVolume()));

	door->GetPhysicsObject()->SetInverseMass(0.0f);
	door->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(door);

	GameObject* hinge = new GameObject("Hinge");

	Matrix3 doorDirection = Matrix3(object.orientation);
	Vector3 hingePosition = object.position + doorDirection * (Vector3(0, 0, object.scale.z/2 + 0.75f));
	AABBVolume* volume2 = new AABBVolume(Vector3(0.5f, object.scale.y, 0.5f)/2);
	hinge->SetBoundingVolume((CollisionVolume*)volume2);
	hinge->GetTransform()
		.SetScale(Vector3(0.5f, object.scale.y, 0.5f))
		.SetPosition(hingePosition)
		.SetOrientation(object.orientation);

	hinge->SetRenderObject(new RenderObject(&hinge->GetTransform(), cubeMesh, basicTex, basicShader));
	hinge->SetPhysicsObject(new PhysicsObject(&hinge->GetTransform(), hinge->GetBoundingVolume()));

	hinge->GetPhysicsObject()->SetInverseMass(0.0f);
	hinge->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(hinge);
	door->SetHinge(hinge);

	HingeConstraint* constraint = new HingeConstraint(door, hinge);
	world.AddConstraint(constraint);
	PositionConstraint* posConst = new PositionConstraint(hinge, door, object.scale.z / 2 + 0.75);
	world.AddConstraint(posConst);
}

void Level::CreateButton(LevelItem object, GameWorld& world) {
	ButtonObject* button = new ButtonObject(object.scale.x);

	Vector3 dimensions = Vector3(0.25f, object.scale.y, object.scale.z);
	OBBVolume* volume = new OBBVolume(dimensions / 2);
	button->SetBoundingVolume((CollisionVolume*)volume);
	button->GetTransform()
		.SetScale(dimensions)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	button->SetRenderObject(new RenderObject(&button->GetTransform(), cubeMesh, basicTex, basicShader));
	button->GetRenderObject()->SetColour(GetButtonColour(object.scale.x));
	button->SetPhysicsObject(new PhysicsObject(&button->GetTransform(), button->GetBoundingVolume()));

	button->GetPhysicsObject()->SetInverseMass(0.0f);
	button->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(button);
}

void Level::CreateEnemy(LevelItem object, GameWorld& world) {
	EnemyObject* enemy = new EnemyObject();

	SphereVolume* volume = new SphereVolume(object.scale.x);
	enemy->SetBoundingVolume((CollisionVolume*)volume);

	enemy->GetTransform()
		.SetScale(object.scale)
		.SetPosition(object.position)
		.SetOrientation(object.orientation);

	enemy->SetRenderObject(new RenderObject(&enemy->GetTransform(), enemyMesh, nullptr, basicShader));
	enemy->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	enemy->SetPhysicsObject(new PhysicsObject(&enemy->GetTransform(), enemy->GetBoundingVolume()));

	enemy->GetPhysicsObject()->SetInverseMass(0.5f);
	enemy->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(enemy);
}

Vector4 Level::GetButtonColour(int id) {
	switch (id) {
	case 0:
		return Vector4(1, 0, 0, 1);
	case 1:
		return Vector4(0, 0, 1, 1);
	case 2:
		return Vector4(1, 1, 0, 1);
	case 3:
		return Vector4(0.25f, 0.17f, 0.08f, 1);
	}
	return Vector4(1, 1, 1, 1);
}
