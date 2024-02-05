#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "HingeConstraint.h"
#include "StateGameObject.h"

#include "PlayerObject.h"
#include "BombObject.h"
#include "ExplosiveObject.h"
#include "HomeObject.h"
#include "DoorObject.h"
#include "ButtonObject.h"
#include "EnemyObject.h"

#include "MainMenuState.h"
#include "SetupNetworkingState.h"
#include "ServerState.h"
#include "PlayingState.h"
#include "PauseState.h"
#include "ShowHighScoresState.h"

#include <filesystem>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world = new GameWorld();
	machine = new PushdownMachine(new SetupNetworkingState());
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	movementMagnitude = 1.0f;
	rotationMagnitude = 1.0f;
	scaleMagnitude = 1.0f;
	useGravity = false;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}

void TutorialGame::ReceivePacket(int type, GamePacket* payload, int source) {
	if (isClient) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			std::string msg = realPacket->GetStringFromData();

			std::cout << "Client recieved message: " << msg << "\n";

			if (msg.substr(0, 2) == "NM") {
				int count = 3;
				std::string tempStr = "";
				int index = 0;
				int total = 0;
				while (msg[count] != ' ') {
					tempStr.push_back(msg[count]);
					count++;
				}
				index = stoi(tempStr);
				count++;
				tempStr = "";
				while (msg[count] != ' ') {
					tempStr.push_back(msg[count]);
					count++;
				}
				total = stoi(tempStr);
				count++;
				if (level.LoadNavMeshNetwork(msg.substr(count), index, total)) {
					NavigationGrid grid = level.GetGridOnline();
					NavigationPath outPath;

					Vector3 startPos(-290, 0, -290);
					Vector3 endPos(190, 0, 192);

					bool found = grid.FindPath(startPos, endPos, outPath);

					Vector3 pos;
					while (outPath.PopWaypoint(pos)) testNodes.push_back(pos);
				}
			}
			else {
				int count = 0;
				std::string expItems = "";
				while (msg[count] != ' ') {
					expItems.push_back(msg[count]);
					count++;
				}
				expectedItems = stoi(expItems);
				count++;
				level.LoadItemFromServer(msg.substr(count));
			}
		}
		else if (type == Int_Message) {
			IntPacket* realPacket = (IntPacket*)payload;

			int msg = realPacket->GetIntFromData();

			std::cout << "Client recieved message: " << msg << "\n";

			ShowHighScoresState* hsState = (ShowHighScoresState*)machine->GetCurrentState();
			hsState->AddHighScore(msg);
		}
	}
	else {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			std::string msg = realPacket->GetStringFromData();

			if (msg.substr(0, 6) == "Levels") {
				clientLevelNames[source] = msg.substr(7);
			}
			else if (msg.substr(0, 2) == "NM") {
				int count = 3;
				std::string tempStr = "";
				int index = 0;
				int total = 0;
				while (msg[count] != ' ') {
					tempStr.push_back(msg[count]);
					count++;
				}
				index = stoi(tempStr);
				count++;
				tempStr = "";
				while (msg[count] != ' ') {
					tempStr.push_back(msg[count]);
					count++;
				}
				total = stoi(tempStr);
				count++;
				level.WriteNavMeshNetwork(msg.substr(count), clientLevelNames[source], index, total);
			}
			else {
				int count = 0;
				std::string tempStr = "";
				while (msg[count] != ' ') {
					tempStr.push_back(msg[count]);
					count++;
				}
				if (expectedItems == INT_MAX) level.ClearLevelFileRaw();
				expectedItems = stoi(tempStr);
				count++;
				tempStr = "";
				while (msg[count] != ',') {
					tempStr.push_back(msg[count]);
					count++;
				}
				clientLevelNames[source] = tempStr;
				count++;
				if (level.WriteItemNetwork(msg.substr(count), clientLevelNames[source], expectedItems)) expectedItems == INT_MAX;
			}

			std::cout << "Server recieved message: " << msg << "\n";
		}
		else if (type == Int_Message) {
			IntPacket* realPacket = (IntPacket*)payload;

			int msg = realPacket->GetIntFromData();

			std::cout << "Server recieved message: " << msg << "\n";

			level.RecordHighScore(clientLevelNames[source], msg);
		}
		else if (type == Data_Request_Message) {
			DataRequestPacket* realPacket = (DataRequestPacket*)payload;

			std::cout << "Data Request \n";

			int req = realPacket->GetRequestFromData();

			std::string data = realPacket->GetExtraData();

			std::vector<std::string> levelFile;

			switch (req) {
			case 0:
				std::cout << "Server recieved request for high scores of " << data << "\n";
				clientLevelNames[source] = data;
				highScores = level.GetHighScores(clientLevelNames[source]);
				for (int i = 0; i < highScores.size(); i++) {
					IntPacket p(highScores[i]);
					server->SendPacketToPeer(p, source);
				}
				break;
			case 1:
				std::cout << "Server recieved reqest to load level " << data << "\n";
				clientLevelNames[source] = data;
				levelFile = level.GetLevelObjectFile(clientLevelNames[source]);
				for (int i = 0; i < levelFile.size(); i++) {
					StringPacket p(std::to_string(levelFile.size()) + " " + levelFile[i]);
					server->SendPacketToPeer(p, source);
				}
				break;
			case 2:
				std::cout << "Server recieved request to load nav grid from level " << data << "\n";
				clientLevelNames[source] = data;
				levelFile = level.GetLevelNavMeshFile(clientLevelNames[source]);
				for (int i = 0; i < levelFile.size(); i++) {
					StringPacket p("NM " + std::to_string(i) + " " + std::to_string(levelFile.size()) + " " + levelFile[i]);
					server->SendPacketToPeer(p, source);
				}
				break;
			}
		}
	}
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("Keeper.msh");
	enemyMesh = renderer->LoadMesh("goat.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	level = Level();
	level.LoadRenderStuff(capsuleMesh, cubeMesh, sphereMesh, charMesh, enemyMesh, basicTex, basicShader);
	InitCamera();
}

void TutorialGame::InitialiseClient() {
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	client = new GameClient();
	client->RegisterPacketHandler(String_Message, this);
	client->RegisterPacketHandler(Int_Message, this);
	client->RegisterPacketHandler(Data_Request_Message, this);
	client->Connect(127, 0, 0, 1, port);
	isClient = true;
	networkingSetUp = true;
}

void TutorialGame::InitialiseServer() {
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	server = new GameServer(port, 4);
	server->RegisterPacketHandler(String_Message, this);
	server->RegisterPacketHandler(Int_Message, this);
	server->RegisterPacketHandler(Data_Request_Message, this);
	networkingSetUp = true;
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
	delete machine;
	NetworkBase::Destroy();
}

bool TutorialGame::UpdateGame(float dt) {
	if (!machine->Update(dt)) return false;
	if (!networkingSetUp) {
		auto* serverState = dynamic_cast<ServerState*>(machine->GetCurrentState());
		if (serverState) {
			InitialiseServer();
		}
		else {
			auto* clientState = dynamic_cast<MainMenuState*>(machine->GetCurrentState());
			if (clientState) {
				if (clientState->IsOffline()) {
					offline = true;
					networkingSetUp = true;
				}
				else InitialiseClient();
			}
		}
	}
	auto* ps = dynamic_cast<PlayingState*>(machine->GetCurrentState());
	if (ps) {
		std::string lName = ps->LoadLevel();
		if (lName != "Loaded") {
			if (ps->IsEditing()) {
				editing = true;
				keyType = KeyType::LevelEditor;
				useGravity = false;
				physics->UseGravity(useGravity);
			}
			else {
				editing = false;
				keyType = KeyType::Player;
				useGravity = true;
				physics->UseGravity(useGravity);
				maxTime = 300;
				timeLeft = 300;
			}
			if (lName == "") {
				InitWorld();
				levelName = "";
				if (!editing) lockedObject = world->GetPlayer();
				levelLoaded = true;
			}
			else {
				levelName = " ";
				InitWorld();
				if (!offline) {
					level.ClearObjects();
					DataRequestPacket p1("1" + lName.substr(7));
					client->SendPacket(p1);
					DataRequestPacket p2("2" + lName.substr(7));
					client->SendPacket(p2);
				}
				else {
					level.LoadLevel(*world, lName);
					if (!editing) lockedObject = world->GetPlayer();
				}
				levelName = lName.substr(7);
			}
		}
		if (!offline && !levelLoaded) {
			if (!level.LoadFullLevel(*world, expectedItems)) {
				client->UpdateClient();
				return true;
			}
			else {
				if(!editing) lockedObject = world->GetPlayer();
				levelLoaded = true;
			}
		}
		for (int i = 1; i < testNodes.size(); i++) {
			Vector3 a = testNodes[i - 1];
			Vector3 b = testNodes[i];

			Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
		}
		if (!inSelectionMode && !writingLevel && !loadingLevel) {
			world->GetMainCamera().UpdateCamera(dt);
		}
		if (writingLevel) {
			count++;
			if (count == 51) count = 0;
		}
		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();

			Vector3 camPos = objPos + (lockedPitch * Vector3(0, 14, 20));

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera().SetPosition(camPos);
			world->GetMainCamera().SetPitch(angles.x);
			world->GetMainCamera().SetYaw(angles.y);

			world->GetPlayer()->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, world->GetMainCamera().GetYaw(), 0));
		}

		if (!editing) {
			timeLeft -= dt;
			Debug::Print(std::format("Time: {0:.0f}", timeLeft), Vector2(80, 15));
			if (world->IsGameOver() || timeLeft <= 0) {
				PlayerObject* p = (PlayerObject*)world->GetPlayer();
				if (p->IsPlayerDead() || timeLeft == 0) {
					ps->Loss();
				}
				else {
					int score = (timeLeft * 10) + (p->GetCollectedGoals() * 300) - ((world->GetGoalTotal() - p->GetCollectedGoals()) * 400);
					if (score < 0) score = 0;
					if (!offline) {
						IntPacket p1(score);
						client->SendPacket(p1);
					}
					else level.RecordHighScore(levelName, score);
					ps->Victory(p->GetCollectedGoals(), world->GetGoalTotal(), timeLeft, maxTime);
				}
			}
		}

		UpdateKeys();

		if (keyType == KeyType::Tutorial) {
			if (useGravity) {
				Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
			}
			else {
				Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
			}
		}
		if (keyType == KeyType::LevelEditor) {
			Debug::DrawCube(Vector3(100, 50, 100), Vector3(0, 50, 0));
		}

		RayCollision closestCollision;
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
			Vector3 rayPos;
			Vector3 rayDir;

			rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

			rayPos = selectionObject->GetTransform().GetPosition();

			Ray r = Ray(rayPos, rayDir);

			if (world->Raycast(r, closestCollision, true, selectionObject)) {
				if (objClosest) {
					objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				}
				objClosest = (GameObject*)closestCollision.node;

				objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
			}
		}

		Debug::DrawAxisLines(Matrix4(), 50.0f);

		if (keyType == KeyType::Tutorial || keyType == KeyType::LevelEditor) SelectObject();
		if (keyType == KeyType::Tutorial) MoveSelectedObject();

		if (testStateObject) testStateObject->Update(dt);

		world->UpdateWorld(dt, keyType == KeyType::Player);
		renderer->Update(dt);
		physics->Update(dt);
	}
	else {
		auto* highScoreState = dynamic_cast<ShowHighScoresState*>(machine->GetCurrentState());
		if (highScoreState) {
			if(levelName == "") levelName = highScoreState->GetLevelName().substr(7);
			if (highScoreState->NeedsHighScores()) {
				if (!offline) {
					DataRequestPacket p("0" + levelName);
					client->SendPacket(p);
				}
				else {
					highScores = level.GetHighScores(levelName);
					for (int i = 0; i < highScores.size(); i++) {
						highScoreState->AddHighScore(highScores[i]);
					}
				}
			}
		}
		auto* pause = dynamic_cast<PauseState*>(machine->GetCurrentState());
		if (!pause) {
			world->ClearAndErase();
			levelLoaded = false;
			expectedItems = INT_MAX;
			if(server == nullptr) levelName = "";
			lockedObject = nullptr;
		}
	}
	renderer->Render();
	Debug::UpdateRenderables(dt);
	if (isClient) client->UpdateClient();
	else if(server != nullptr) server->UpdateServer();
	return true;
}

void TutorialGame::UpdateKeys() {
	switch (keyType) {
	case KeyType::Tutorial:
		UpdateTutorialKeys();
		break;
	case KeyType::LevelEditor:
		UpdateLevelEditorKeys();
		break;
	case KeyType::Player:
		UpdatePlayerKeys();
		break;
	}
}

void TutorialGame::UpdateTutorialKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		keyType = KeyType::LevelEditor;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::UpdateLevelEditorKeys() {
	if (writingLevel) {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::BACK) && levelName.length() > 0) {
			levelName.pop_back();
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN) && levelName.length() > 0) {
			if (!offline) {
				std::vector<std::string> items = level.GetLevelObjectWorld(*world);
				if (!items.empty()) {
					for (int i = 0; i < items.size(); i++) {
						StringPacket p(std::to_string(items.size()) + " " + levelName + "," + items[i]);
						client->SendPacket(p);
					}
					items = level.GetLevelNavMesh(*world);
					for (int i = 0; i < items.size(); i++) {
						StringPacket p("NM " + std::to_string(i) + " " + std::to_string(items.size()) + " " + items[i]);
						client->SendPacket(p);
					}
				}
			}
			else {
				level.WriteLevel(*world, levelName);
			}
			writingLevel = false;
			count = 0;
		}
		else {
			char key = Window::GetKeyboard()->KeyTyped();
			if (key) levelName.push_back(key);
		}
		Debug::Print("Name This Level", Vector2(30, 30), Vector4(1, 1, 1, 1), 30);
		if (count < 25) {
			Debug::Print(levelName + "_", Vector2(50 - ((float)levelName.length()/1.13f), 45));
		}
		else {
			Debug::Print(levelName, Vector2(50 - ((float)levelName.length()/1.13f), 45));
		}
	}
	else if (loadingLevel) {
		if (levels.size() > 9) {
			int scroll = -Window::GetMouse()->GetWheelMovement();
			count = std::min((int)levels.size()-9, std::max(0, count + scroll));
		}
		Debug::Print("Choose a level", Vector2(30, 20), Vector4(1, 1, 1, 1), 30.0f);
		for (int i = count; i < count + 9 && i < levels.size(); i++) {
			Debug::Print(std::to_string(i + 1 - count) + ": " + levels[i].substr(7), Vector2(30, 30 + ((i - count) * 3)));
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) { 
			InitWorld();
			level.LoadLevel(*world, levels[count]);
			levelName = levels[count].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2) && levels.size() > count + 1) { 
			InitWorld(); 
			level.LoadLevel(*world, levels[count + 1]);
			levelName = levels[count+1].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3) && levels.size() > count + 2) { 
			InitWorld();
			level.LoadLevel(*world, levels[count+2]);
			levelName = levels[count+2].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM4) && levels.size() > count + 3) {
			InitWorld();
			level.LoadLevel(*world, levels[count+3]);
			levelName = levels[count+3].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM5) && levels.size() > count + 4) { 
			InitWorld();
			level.LoadLevel(*world, levels[count+4]);
			levelName = levels[count+4].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM6) && levels.size() > count + 5) { 
			InitWorld();
			level.LoadLevel(*world, levels[count+5]);
			levelName = levels[count+5].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM7) && levels.size() > count + 6) { 
			InitWorld();
			level.LoadLevel(*world, levels[count+6]);
			levelName = levels[count+6].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM8) && levels.size() > count + 7) { 
			InitWorld();
			level.LoadLevel(*world, levels[count+7]);
			levelName = levels[count+7].substr(7);
			loadingLevel = false;
			count = 0;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM9) && levels.size() > count + 8) { 
			InitWorld();
			level.LoadLevel(*world, levels[count+8]);
			levelName = levels[count+8].substr(7);
			loadingLevel = false;
			count = 0;
		}
	}
	else {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
			smoothObjectMovement = !smoothObjectMovement;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
			editorTool = (EditorTool)((int)editorTool << 1);
			if (editorTool == EditorTool::Max) editorTool = EditorTool::Move;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
			showBoundingVolume = !showBoundingVolume;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F4)) {
			levelName.clear();
			InitWorld();
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F5)) {
			writingLevel = true;
		}
		/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
			count = 0;
			levels.clear();
			for (const auto& entry : std::filesystem::directory_iterator("Levels")) levels.push_back(entry.path().string());
			loadingLevel = true;
			showBoundingVolume = false;
		}*/
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F6)) {
			keyType = KeyType::Player;
			lockedObject = world->GetPlayer();
			useGravity = true;
			physics->UseGravity(useGravity);
			lockedPitch = world->GetPlayer()->GetTransform().GetOrientation();
		}
		if (smoothObjectMovement) Debug::Print("F1: Lockstep Movement", Vector2(0, 3), Vector4(1, 1, 1, 1));
		else Debug::Print("F1: Unlocked Movement", Vector2(0, 3), Vector4(1, 1, 1, 1));
		Debug::Print("F2: Switch Mode", Vector2(0, 6), Vector4(1, 1, 1, 1));
		Debug::Print("F3: Show Bounding Volume", Vector2(0, 9), Vector4(1, 1, 1, 1));
		Debug::Print("F4: Reset World", Vector2(0, 12), Vector4(1, 1, 1, 1));
		Debug::Print("F5: Save Level", Vector2(0, 15), Vector4(1, 1, 1, 1));
		//Debug::Print("F7: Load Level", Vector2(0, 21), Vector4(1, 1, 1, 1));
		Debug::Print("F6: Test Level", Vector2(0, 18), Vector4(1, 1, 1, 1));
		int scroll;
		switch (editorTool) {
		case EditorTool::Move:
			Debug::Print("Current Mode: Move", Vector2(50, 6), Vector4(1, 1, 1, 1));
			Debug::Print("Move Magnitude:" + std::format("{0:.1f}", movementMagnitude), Vector2(5, 90));
			movementMagnitude += Window::GetMouse()->GetWheelMovement() * 0.1f;
			if (inSelectionMode && selectionObject) {
				Debug::Print(std::format("({0:.1f}, {1:.1f}, {2:.1f})", selectionObject->GetTransform().GetPosition().x,
					selectionObject->GetTransform().GetPosition().y,
					selectionObject->GetTransform().GetPosition().z),
					Vector2(50, 3), Vector4(1, 1, 1, 1));
			}
			break;
		case EditorTool::Rotate:
			Debug::Print("Current Mode: Rotate", Vector2(50, 6), Vector4(1, 1, 1, 1));
			Debug::Print("Rotation Magnitude: " + std::format("{0:.0f}", rotationMagnitude), Vector2(5, 90));
			rotationMagnitude += Window::GetMouse()->GetWheelMovement();
			if (inSelectionMode && selectionObject) {
				Vector3 orientation = selectionObject->GetTransform().GetOrientation().ToEuler();
				Debug::Print(std::format("({0:.1f}, {1:.1f}, {2:.1f})", orientation.x,
					orientation.y,
					orientation.z),
					Vector2(50, 3), Vector4(1, 1, 1, 1));
			}
			break;
		case EditorTool::Scale:
			Debug::Print("Current Mode: Scale", Vector2(50, 6), Vector4(1, 1, 1, 1));
			Debug::Print("Scale Magnitude: " + std::format("{0:.1f}", scaleMagnitude), Vector2(5, 90));
			scaleMagnitude += Window::GetMouse()->GetWheelMovement() * 0.1f;
			if (inSelectionMode && selectionObject) {
				if (selectionObject->GetName() == "Button") {
					ButtonObject* obj = (ButtonObject*)selectionObject;
					Debug::Print(std::format("(0.25, {0:.1f}, {1:.1f})",
						selectionObject->GetTransform().GetScale().y,
						selectionObject->GetTransform().GetScale().z),
						Vector2(50, 3), Vector4(1, 1, 1, 1));
					Debug::Print(std::format("Button ID: {}", obj->GetButtonID()), Vector2(50, 12), Vector4(1, 1, 1, 1));
				}
				else if (selectionObject->GetName() == "Door") {
					DoorObject* obj = (DoorObject*)selectionObject;
					Debug::Print(std::format("(1.0, {0:.1f}, {1:.1f})",
						selectionObject->GetTransform().GetScale().y,
						selectionObject->GetTransform().GetScale().z),
						Vector2(50, 3), Vector4(1, 1, 1, 1));
					Debug::Print(std::format("Button ID: {}", obj->GetButton()), Vector2(50, 12), Vector4(1, 1, 1, 1));
				}
				else {
					Debug::Print(std::format("({0:.1f}, {1:.1f}, {2:.1f})", selectionObject->GetTransform().GetScale().x,
						selectionObject->GetTransform().GetScale().y,
						selectionObject->GetTransform().GetScale().z),
						Vector2(50, 3), Vector4(1, 1, 1, 1));
				}
			}
			break;
		case EditorTool::AddObject:
			scroll = -Window::GetMouse()->GetWheelMovement();
			count = std::min(5, std::max(0, count + scroll));
			Debug::Print("Current Mode: Add Objects", Vector2(50, 6), Vector4(1, 1, 1, 1));
			for (int i = 0; i < 5; i++) {
				std::string str = GetAddName(count + i);
				Debug::Print("Add " + str, Vector2(8+(i*20) - (str.size()/1.15f), 90), Vector4(1, 1, 1, 1), 15.0f);
				Debug::Print(std::to_string(i + 1), Vector2(8 + (i * 20), 93));
			}
			break;
		case EditorTool::ChangeMass:
			Debug::Print("Current Mode: Change Mass", Vector2(50, 6), Vector4(1, 1, 1, 1));
			if (inSelectionMode && selectionObject) {
				Debug::Print("Inverse Mass: " + std::format("{0:.1f}", selectionObject->GetPhysicsObject()->GetInverseMass()), Vector2(50, 3));
				float massToAdd = Window::GetMouse()->GetWheelMovement() * 0.1f;
				if (massToAdd + selectionObject->GetPhysicsObject()->GetInverseMass() < 0) selectionObject->GetPhysicsObject()->SetInverseMass(0);
				else selectionObject->GetPhysicsObject()->SetInverseMass(massToAdd + selectionObject->GetPhysicsObject()->GetInverseMass());
			}
		}
		if (inSelectionMode && selectionObject) {
			if (showBoundingVolume) {
				if (selectionObject->GetBoundingVolume()->type == VolumeType::AABB) {
					AABBVolume volume = (AABBVolume&)*selectionObject->GetBoundingVolume();
					Debug::DrawCube(volume.GetHalfDimensions(), selectionObject->GetTransform().GetPosition());
				}
				else if (selectionObject->GetBoundingVolume()->type == VolumeType::OBB) {
					OBBVolume volume = (OBBVolume&)*selectionObject->GetBoundingVolume();
					Debug::DrawRotatedCube(volume.GetHalfDimensions(), selectionObject->GetTransform().GetPosition(), selectionObject->GetTransform().GetOrientation());
				}
				else if (selectionObject->GetBoundingVolume()->type == VolumeType::Sphere) {
					SphereVolume volume = (SphereVolume&)*selectionObject->GetBoundingVolume();
					Debug::DrawSphere(volume.GetRadius(), selectionObject->GetTransform().GetPosition());
				}
				else if (selectionObject->GetBoundingVolume()->type == VolumeType::Capsule) {
					CapsuleVolume volume = (CapsuleVolume&)*selectionObject->GetBoundingVolume();
					Debug::DrawCapsule(volume.GetHalfHeight(), volume.GetRadius(),
						selectionObject->GetTransform().GetPosition(), selectionObject->GetTransform().GetOrientation());
				}
			}
			if (smoothObjectMovement) {
				if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
					MoveLevelEditor(world->GetMainCamera().GetUp(), world->GetMainCamera().GetForward());
				}
				if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
					MoveLevelEditor(-world->GetMainCamera().GetUp(), -world->GetMainCamera().GetForward());
				}
				if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
					MoveLevelEditor(world->GetMainCamera().GetRight(), world->GetMainCamera().GetUp());
				}
				if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
					MoveLevelEditor(-world->GetMainCamera().GetRight(), -world->GetMainCamera().GetUp());
				}
				if (Window::GetKeyboard()->KeyDown(KeyCodes::E) && editorTool == EditorTool::Scale) {
					ScaleObject(Vector3(1, 1, 1));
				}
				if (Window::GetKeyboard()->KeyDown(KeyCodes::S) && editorTool == EditorTool::Scale) {
					ScaleObject(Vector3(-1, -1, -1));
				}
			}
			else {
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP)) {
					MoveLevelEditor(world->GetMainCamera().GetUp(), world->GetMainCamera().GetForward());
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
					MoveLevelEditor(-world->GetMainCamera().GetUp(), -world->GetMainCamera().GetForward());
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::RIGHT)) {
					MoveLevelEditor(world->GetMainCamera().GetRight(), world->GetMainCamera().GetUp());
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::LEFT)) {
					MoveLevelEditor(-world->GetMainCamera().GetRight(), -world->GetMainCamera().GetUp());
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::E) && editorTool == EditorTool::Scale) {
					ScaleObject(Vector3(1, 1, 1));
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::S) && editorTool == EditorTool::Scale) {
					ScaleObject(Vector3(-1, -1, -1));
				}
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
				selectionObject->GetTransform().SetOrientation(Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::DELETEKEY)) {
				world->RemoveGameObject(selectionObject, true);
				selectionObject = nullptr;
			}
			if (editorTool == EditorTool::ChangeMass && Window::GetKeyboard()->KeyPressed(KeyCodes::Z)) {
				selectionObject->GetPhysicsObject()->SetInverseMass(0);
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::C)) {
				if (selectionObject->GetBoundingVolume()->type == VolumeType::AABB && selectionObject->GetName() == "" || selectionObject->GetName() == "Wall") {
					AddCubeToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10), 
						selectionObject->GetTransform().GetScale()/2, selectionObject->GetPhysicsObject()->GetInverseMass());
				}
				else if (selectionObject->GetBoundingVolume()->type == VolumeType::Sphere && selectionObject->GetName() == "") {
					AddSphereToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10),
						selectionObject->GetTransform().GetScale().x, selectionObject->GetPhysicsObject()->GetInverseMass());
				}
			}
		}
		else {
			if (editorTool == EditorTool::AddObject) {
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
					AddToWorld(count);
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2)) {
					AddToWorld(count + 1);
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3)) {
					AddToWorld(count + 2);
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM4)) {
					AddToWorld(count + 3);
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM5)) {
					AddToWorld(count + 4);
				}
			}
		}
	}
}

void TutorialGame::UpdatePlayerKeys() {
	LockedObjectMovement();
	PlayerObject* p = (PlayerObject*)world->GetPlayer();
	switch (p->GetSelected()) {
	case 0:
		Debug::Print(std::format(">Bombs: {}", p->GetBombCount()), Vector2(80, 40), Vector4(1, 1, 1, 1), 15.0f);
		Debug::Print(std::format("C4: {}", p->GetC4Count()), Vector2(80, 43), Vector4(1, 1, 1, 1), 15.0f);
		break;
	case 1:
		Debug::Print(std::format("Bombs: {}", p->GetBombCount()), Vector2(80, 40), Vector4(1, 1, 1, 1), 15.0f);
		Debug::Print(std::format(">C4: {}", p->GetC4Count()), Vector2(80, 43), Vector4(1, 1, 1, 1), 15.0f);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1) && editing) {
		lockedObject = nullptr;
		useGravity = false;
		physics->UseGravity(useGravity);
		keyType = KeyType::Tutorial;
		InitWorld();
		keyType = KeyType::LevelEditor;
		if (levelName != "") {
			level.LoadFullLevel(*world, expectedItems);
		}
	}
}

void TutorialGame::MoveLevelEditor(const Vector3& moveDirection, const Vector3& rotateDirection) {
	switch (editorTool) {
	case EditorTool::Move:
		selectionObject->GetTransform().SetPosition(selectionObject->GetTransform().GetPosition()
			+ (moveDirection * movementMagnitude));
		break;
	case EditorTool::Rotate:
		selectionObject->GetTransform().SetOrientation(selectionObject->GetTransform().GetOrientation() *
			Quaternion(sin((rotationMagnitude * rotateDirection.x * (PI / 180)) / 2), sin((rotationMagnitude * rotateDirection.y * (PI / 180)) / 2),
				sin((rotationMagnitude * rotateDirection.z * (PI / 180)) / 2), cos((rotationMagnitude * (PI/180)) / 2)));
		break;
	case EditorTool::Scale:
		ScaleObject(moveDirection);
		break;
	}
}

void TutorialGame::ScaleObject(const Vector3& scaleValue) {
	float scaleAmount = scaleValue.x + scaleValue.y + scaleValue.z;
	if (abs(scaleAmount) != 1) scaleAmount = scaleAmount / abs(scaleAmount);
	Vector3 scaleOffset = Vector3(0, 0, 0);
	switch (selectionObject->GetBoundingVolume()->type) {
	case VolumeType::AABB:
		if (selectionObject->GetName() == "Button") {
			if (scaleValue.x > 0) {
				ButtonObject* obj = (ButtonObject*)selectionObject;
				obj->SetButtonID(obj->GetButtonID() + 1);
				selectionObject->GetRenderObject()->SetColour(level.GetButtonColour(obj->GetButtonID()));
				scaleOffset.x = scaleValue.x;
			}
			else if (scaleValue.x < 0) {
				ButtonObject* obj = (ButtonObject*)selectionObject;
				obj->SetButtonID(obj->GetButtonID() - 1);
				selectionObject->GetRenderObject()->SetColour(level.GetButtonColour(obj->GetButtonID()));
				scaleOffset.x = scaleValue.x;
			}
		}
		selectionObject->GetTransform().SetScale(selectionObject->GetTransform().GetScale()
			+ ((scaleValue-scaleOffset) * scaleMagnitude));
		ScaleAABB((AABBVolume&)*selectionObject->GetBoundingVolume(), scaleValue-scaleOffset);
		break;
	case VolumeType::OBB:
		if (selectionObject->GetName() == "Door") {
			if (scaleValue.x > 0) {
				DoorObject* obj = (DoorObject*)selectionObject;
				obj->SetButton(obj->GetButton() + 1);
				selectionObject->GetRenderObject()->SetColour(level.GetButtonColour(obj->GetButton()));
				selectionObjectColour = selectionObject->GetRenderObject()->GetColour();
				scaleOffset.x = scaleValue.x;
			}
			else if (scaleValue.x < 0) {
				DoorObject* obj = (DoorObject*)selectionObject;
				obj->SetButton(obj->GetButton() - 1);
				selectionObject->GetRenderObject()->SetColour(level.GetButtonColour(obj->GetButton()));
				selectionObjectColour = selectionObject->GetRenderObject()->GetColour();
				scaleOffset.x = scaleValue.x;
			}
		}
		selectionObject->GetTransform().SetScale(selectionObject->GetTransform().GetScale()
			+ ((scaleValue-scaleOffset) * scaleMagnitude));
		ScaleOBB((OBBVolume&)*selectionObject->GetBoundingVolume(), scaleValue-scaleOffset);
		break;
	case VolumeType::Sphere:
		selectionObject->GetTransform().SetScale(selectionObject->GetTransform().GetScale()
			+ (Vector3(scaleAmount, scaleAmount, scaleAmount) * scaleMagnitude));
		ScaleSphere((SphereVolume&)*selectionObject->GetBoundingVolume(), scaleAmount);
		break;
	case VolumeType::Capsule:
		scaleAmount = scaleValue.z + scaleValue.z;
		if (abs(scaleAmount) != 1 && abs(scaleAmount) != 0) scaleAmount = scaleAmount / abs(scaleAmount);
		selectionObject->GetTransform().SetScale(selectionObject->GetTransform().GetScale() + 
			(Vector3(scaleAmount, scaleValue.y, scaleAmount) * scaleMagnitude));
		ScaleCapsule((CapsuleVolume&)*selectionObject->GetBoundingVolume(), scaleValue);
		break;
	}
}

void TutorialGame::ScaleAABB(AABBVolume& volume, const Vector3& scaleValue) {
	volume.SetHalfDimensions(volume.GetHalfDimensions() + ((scaleValue*scaleMagnitude)/2));
}

void TutorialGame::ScaleOBB(OBBVolume& volume, const Vector3& scaleValue) {
	volume.SetHalfDimensions(volume.GetHalfDimensions() + ((scaleValue * scaleMagnitude) / 2));
}

void TutorialGame::ScaleSphere(SphereVolume& volume, float scaleValue) {
	volume.SetRadius(volume.GetRadius() + ((scaleValue * scaleMagnitude)));
}

void TutorialGame::ScaleCapsule(CapsuleVolume& volume, const Vector3& scaleValue) {
	volume.SetHalfHeight(volume.GetHalfHeight() + ((scaleValue.y * scaleMagnitude)/2));
	float scaleAmount = scaleValue.x + scaleValue.z;
	if (abs(scaleAmount) != 1 && abs(scaleAmount) != 0) scaleAmount = scaleAmount / abs(scaleAmount);
	volume.SetRadius(volume.GetRadius() + ((scaleAmount * scaleMagnitude)/2));
}

std::string TutorialGame::GetAddName(int i) {
	switch (i) {
	case 0:
		return "Sphere";
	case 1:
		return "Cube";
	case 2:
		return "Oriented Cube";
	case 3:
		return "Player";
	case 4:
		return "Bomb";
	case 5:
		return "Goal";
	case 6:
		return "Home";
	case 7:
		return "Door";
	case 8:
		return "Button";
	case 9:
		return "Enemy";
	}
	return "";
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	float xSpin = world->GetMainCamera().GetController().GetNamedAxis("XLook");
	float ySpin = world->GetMainCamera().GetController().GetNamedAxis("YLook");
	if (lockedPitch.ToEuler().x - ySpin <= 40 && lockedPitch.ToEuler().x - ySpin >= -20) {
		lockedPitch = Matrix4::Rotation(-ySpin, world->GetMainCamera().GetRight180()) * Matrix4::Rotation(-xSpin, Vector3(0, 1, 0)) * lockedPitch;
	}
	else {
		lockedPitch = Matrix4::Rotation(-xSpin, Vector3(0, 1, 0)) * lockedPitch;
	}
	lockedPitch.Normalise();
	PlayerObject* obj = dynamic_cast<PlayerObject*>(lockedObject);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis*3);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis*3);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		lockedObject->GetPhysicsObject()->AddForce(rightAxis*3);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis*3);
	}

	if (obj) {
		obj->ScrollSelected(-Window::GetMouse()->GetWheelMovement());
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			obj->JumpInput();
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::E)) {
			obj->Interact(*world);
		}
		if (Window::GetMouse()->ButtonPressed(MouseButtons::Left)) {
			Vector3 dir, position;
			AABBVolume* volume;
			GameObject* wall;
			switch (obj->UseItem()) {
			case 0:
				dir = (obj->GetTransform().GetPosition() - world->GetMainCamera().GetPosition()).Normalised();

				ThrowBomb(obj->GetTransform().GetPosition() + (dir * 5), dir, 5.0f);
				break;
			case 1:
				wall = obj->GetHoveredObject();
				volume = (AABBVolume*)wall->GetBoundingVolume();
				position = wall->GetTransform().GetPosition() + (-world->GetMainCamera().GetForward180() * volume->GetHalfDimensions());
				position.y = std::min(position.y, world->GetMainCamera().GetPosition().y);
				obj->AddC4((ExplosiveObject*)PlaceC4(position, -world->GetMainCamera().GetForward180(), wall));
				break;
			case 2:
				vector<Vector3> c4List = obj->GetExplodedC4Positions();
				vector<Transform> wallList = obj->GetExplodedWallPositions();
				for (int i = 0; i < c4List.size(); i++) {
					ThrowBomb(c4List[i] + Vector3(c4List[i].x - wallList[i].GetPosition().x, 0, c4List[i].z - wallList[i].GetPosition().z), Vector3(0, 1, 0), 0.0f);
					SplitWall(wallList[i]);
				}
				break;
			}
		}
		if (obj->IsOnFloor() && obj->JumpPressed()) {
			obj->Jump();
		}
	}
	else {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			lockedObject->GetPhysicsObject()->AddForce(Vector3(0, 100, 0));
		}
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	testStateObject = nullptr;
	selectionObject = nullptr;
	world->ClearPlayer();
	if (levelName == "") {
		InitMixedGridWorld(10, 10, 3.5f, 3.5f);
		BridgeConstraintTest();
		testStateObject = AddStateObjectToWorld(Vector3(-10, 10, -10));
		InitGameExamples();
		AddOrientedCubeToWorld(Vector3(20, 0, 20), Vector3(6, 6, 6), 0.5f);
		AddOrientedCubeToWorld(Vector3(20, 0, 50), Vector3(6, 6, 6), 0.5f);
		InitDefaultFloor();
	}
}

GameObject* TutorialGame::AddToWorld(int i) {
	switch (i) {
	case 0:
		return AddSphereToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10), 1);
	case 1:
		return AddCubeToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10), Vector3(1, 1, 1));
	case 2:
		return AddOrientedCubeToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10), Vector3(1, 1, 1));
	case 3:
		return AddPlayerToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	case 4:
		return AddBombToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	case 5:
		return AddGoalToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	case 6:
		return AddHomeToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	case 7:
		return AddDoorToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	case 8:
		return AddButtonToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	case 9:
		return AddEnemyToWorld(world->GetMainCamera().GetPosition() + (world->GetMainCamera().GetCameraDirection() * 10));
	}
	return nullptr;
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("Floor");

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);
	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOrientedCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass)
{
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	PlayerObject* character = new PlayerObject();
	CapsuleVolume* volume = new CapsuleVolume(1.5f, 1.5f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(1, 0.5f, 0, 1));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddPlayer(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 1.5f;
	float inverseMass = 0.5f;

	EnemyObject* character = new EnemyObject();

	SphereVolume* volume = new SphereVolume(meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddBombToWorld(const Vector3& position)
{
	GameObject* bomb = new GameObject("Bomb");

	SphereVolume* volume = new SphereVolume(2.0f);
	bomb->SetBoundingVolume((CollisionVolume*)volume);

	bomb->GetTransform()
		.SetScale(Vector3(2.0f, 2.0f, 2.0f))
		.SetPosition(position);

	bomb->SetRenderObject(new RenderObject(&bomb->GetTransform(), sphereMesh, basicTex, basicShader));
	bomb->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	bomb->SetPhysicsObject(new PhysicsObject(&bomb->GetTransform(), bomb->GetBoundingVolume()));

	bomb->GetPhysicsObject()->SetInverseMass(0.0f);
	bomb->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(bomb);

	return bomb;
}

GameObject* TutorialGame::AddGoalToWorld(const Vector3& position)
{
	GameObject* goal = new GameObject("Goal");

	SphereVolume* volume = new SphereVolume(1.0f);
	goal->SetBoundingVolume((CollisionVolume*)volume);

	goal->GetTransform()
		.SetScale(Vector3(1.0f, 1.0f, 1.0f))
		.SetPosition(position);

	goal->SetRenderObject(new RenderObject(&goal->GetTransform(), sphereMesh, nullptr, basicShader));
	goal->GetRenderObject()->SetColour(Vector4(0.65f, 0.486f, 0, 1));
	goal->SetPhysicsObject(new PhysicsObject(&goal->GetTransform(), goal->GetBoundingVolume()));

	goal->GetPhysicsObject()->SetInverseMass(0.0f);
	goal->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(goal);

	return goal;
}

GameObject* TutorialGame::AddHomeToWorld(const Vector3& position)
{
	HomeObject* home = new HomeObject();

	AABBVolume* volume = new AABBVolume(Vector3(25, 1, 25));
	home->SetBoundingVolume((CollisionVolume*)volume);

	home->GetTransform()
		.SetPosition(position)
		.SetScale(Vector3(50, 2, 50));

	home->SetRenderObject(new RenderObject(&home->GetTransform(), cubeMesh, nullptr, basicShader));
	home->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	home->SetPhysicsObject(new PhysicsObject(&home->GetTransform(), home->GetBoundingVolume()));

	home->GetPhysicsObject()->SetInverseMass(0.0f);
	home->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(home);

	return home;
}

GameObject* TutorialGame::AddDoorToWorld(const Vector3& position)
{
	DoorObject* door = new DoorObject(0);

	Vector3 dimensions = Vector3(0.5f, 8.0f, 5.0f);
	OBBVolume* volume = new OBBVolume(dimensions);
	door->SetBoundingVolume((CollisionVolume*)volume);

	door->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	door->SetRenderObject(new RenderObject(&door->GetTransform(), cubeMesh, basicTex, basicShader));
	door->GetRenderObject()->SetColour(level.GetButtonColour(0));
	door->SetPhysicsObject(new PhysicsObject(&door->GetTransform(), door->GetBoundingVolume()));

	door->GetPhysicsObject()->SetInverseMass(0.0f);
	door->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(door);

	return door;
}

GameObject* TutorialGame::AddButtonToWorld(const Vector3& position)
{
	ButtonObject* button = new ButtonObject(0);

	Vector3 dimensions = Vector3(0.125f, 1.0f, 1.0f);
	OBBVolume* volume = new OBBVolume(dimensions);
	button->SetBoundingVolume((CollisionVolume*)volume);

	button->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	button->SetRenderObject(new RenderObject(&button->GetTransform(), cubeMesh, basicTex, basicShader));
	button->GetRenderObject()->SetColour(level.GetButtonColour(0));
	button->SetPhysicsObject(new PhysicsObject(&button->GetTransform(), button->GetBoundingVolume()));

	button->GetPhysicsObject()->SetInverseMass(0.0f);
	button->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(button);

	return button;
}

GameObject* TutorialGame::ThrowBomb(const Vector3& position, const Vector3& direction, float timeToBlow)
{
	BombObject* bomb = new BombObject(timeToBlow);

	SphereVolume* volume = new SphereVolume(1.0f);
	bomb->SetBoundingVolume((CollisionVolume*)volume);

	bomb->GetTransform()
		.SetScale(Vector3(1.0f, 1.0f, 1.0f))
		.SetPosition(position);

	bomb->SetRenderObject(new RenderObject(&bomb->GetTransform(), sphereMesh, basicTex, basicShader));
	bomb->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	bomb->SetPhysicsObject(new PhysicsObject(&bomb->GetTransform(), bomb->GetBoundingVolume()));

	bomb->GetPhysicsObject()->SetInverseMass(10.0f);
	bomb->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(bomb);

	bomb->GetPhysicsObject()->AddForceAtPosition(direction * 300.0f, position - direction);

	return bomb;
}

GameObject* TutorialGame::PlaceC4(const Vector3& position, const Vector3& directionFromWall, GameObject* wall)
{
	ExplosiveObject* c4 = new ExplosiveObject("C4", wall);

	Vector3 dimensions = Vector3(0.25, 0.25, 0.25) - (directionFromWall.GetAbsVector()*0.15);

	AABBVolume* volume = new AABBVolume(dimensions);
	c4->SetBoundingVolume((CollisionVolume*)volume);

	c4->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2)
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(directionFromWall.x*90, directionFromWall.y*90, directionFromWall.z*90));

	c4->SetRenderObject(new RenderObject(&c4->GetTransform(), cubeMesh, nullptr, basicShader));
	c4->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	c4->SetPhysicsObject(new PhysicsObject(&c4->GetTransform(), c4->GetBoundingVolume()));

	c4->GetPhysicsObject()->SetInverseMass(0);
	c4->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(c4);

	return c4;
}

void TutorialGame::SplitWall(Transform wall) {
	Vector3 dimensions;
	Vector3 position = wall.GetPosition() - (wall.GetScale() / 2);
	if (wall.GetScale().x > wall.GetScale().z) {
		dimensions = Vector3(wall.GetScale().z, wall.GetScale().z, wall.GetScale().z);
		if ((int)wall.GetScale().x % (int)wall.GetScale().z > (int)(wall.GetScale().x / wall.GetScale().z)) {
			int timesIn = (int)(wall.GetScale().x / wall.GetScale().z);
			dimensions.x += (wall.GetScale().x - (dimensions.x * timesIn))/timesIn;
		}
		if ((int)wall.GetScale().y % (int)wall.GetScale().z > dimensions.y) {
			int timesIn = (int)(wall.GetScale().y / wall.GetScale().z);
			dimensions.y += (wall.GetScale().y - (dimensions.y * timesIn)) / timesIn;
		}
		for (int i = 0; i < wall.GetScale().x; i += dimensions.x) {
			for (int j = 0; j < wall.GetScale().y; j += dimensions.y) {
				Vector3 pos = position + Vector3(i, j, 0) + (dimensions / 2);
				AddOrientedCubeToWorld(pos, dimensions/2, 25.0f);
			}
		}
	}
	else if (wall.GetScale().x < wall.GetScale().z) {
		dimensions = Vector3(wall.GetScale().x, wall.GetScale().x, wall.GetScale().x);
		if ((int)wall.GetScale().z % (int)wall.GetScale().x > (int)(wall.GetScale().z / wall.GetScale().x)) {
			int timesIn = (int)(wall.GetScale().z / wall.GetScale().z);
			dimensions.z += (wall.GetScale().z - (dimensions.z * timesIn)) / timesIn;
		}
		if ((int)wall.GetScale().y % (int)wall.GetScale().x > dimensions.y) {
			int timesIn = (int)(wall.GetScale().y / wall.GetScale().x);
			dimensions.y += (wall.GetScale().y - (dimensions.y * timesIn)) / timesIn;
		}
		for (int i = 0; i < wall.GetScale().z; i += dimensions.z) {
			for (int j = 0; j < wall.GetScale().y; j += dimensions.y) {
				Vector3 pos = position + Vector3(0, j, i) + (dimensions / 2);
				AddOrientedCubeToWorld(pos, dimensions / 2, 25.0f);
			}
		}
	}
	else {
		dimensions = Vector3(wall.GetScale().x / 2, wall.GetScale().y / 2, wall.GetScale().z / 2);
		for (int i = 0; i < wall.GetScale().x; i += dimensions.x) {
			for (int j = 0; j < wall.GetScale().y; j += dimensions.y) {
				for (int k = 0; k < wall.GetScale().z; k += dimensions.z) {
					Vector3 pos = position + Vector3(i, j, k) + (dimensions / 2);
					AddOrientedCubeToWorld(pos, dimensions / 2, 25.0f);
				}
			}
		}
	}
	
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	StateGameObject* cube = new StateGameObject();

	AABBVolume* volume = new AABBVolume(Vector3(1.0f, 1.0f, 1.0f));
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(1.0f);
	cube->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::BridgeConstraintTest()
{
	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5;
	int numLinks = 10;
	float maxDistance = 30;
	float cubeDistance = 20;

	Vector3 startPos = Vector3(100, 100, 100);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; i++) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, -15, -5));
	AddEnemyToWorld(Vector3(5, -16, -5));
	//AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(selectionObjectColour);
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				if (selectionObject->GetName() == "Hinge") { selectionObject = nullptr; return false; }
				selectionObjectColour = selectionObject->GetRenderObject()->GetColour();
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
	/*if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::A)) {
		selectionObject->GetPhysicsObject()->AddForceAtPosition(Vector3(-1, 0, 0) * forceMagnitude, selectionObject->GetTransform().GetPosition());
	}*/
}