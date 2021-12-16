#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "..//CSC8503Common/PositionConstraint.h"

#include "../CSC8503Common/StateGameObject.h"
#include "..//CSC8503Common/NavigationGrid.h"
#include "..//CSC8503Common/NavigationMap.h"
#include "..//CSC8503Common/NavigationPath.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	machine = new PushdownMachine(new IntroScreen(this));
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	//useGravity		= false;
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	//InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	//InitWorld();//when initgame1 and initgame2 func done, donot call this func
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	glClearColor(1, 1, 1, 1);
	if (dt == 0) {
		renderer->DrawString("Game Paused (ESC)", Vector2(30, 10));
	}
	else {
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}
		/*
	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}*/
		UpdateKeys();
		SelectObject();
		MoveSelectedObject();
		physics->Update(dt);
		if (lockedObject != nullptr) {
				Vector3 objPos = lockedObject->GetTransform().GetPosition();
				Vector3 camPos = objPos + lockedOffset;

				Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

				Matrix4 modelMat = temp.Inverse();

				Quaternion q(modelMat);
				Vector3 angles = q.ToEuler(); //nearly there now!

				world->GetMainCamera()->SetPosition(camPos);
				world->GetMainCamera()->SetPitch(angles.x);
				world->GetMainCamera()->SetYaw(angles.y);

				//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
			}

		//state machine code begin
		if (testStateObject) {
			testStateObject->Update(dt);
			testStateObject1->Update(dt);
		}
		//state machine code end
		if (pathfound) {
			DisplayPathfinding();//display path on the floor by green line
		//in the maze game, create player update(dt) func
		//in this func, draw the path line with dt and position change
		}
	

		world->UpdateWorld(dt);
		renderer->Update(dt);
	}
	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R) && Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
		InitGameWorld1();
		selectionObject = nullptr;
		lockedObject = nullptr;
		physics->SetNum();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R) && Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
		InitGameWorld2();
		selectionObject = nullptr;
		lockedObject = nullptr;
		physics->SetNum();
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	physics->UseGravity(true);
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	//world->GetMainCamera()->SetPosition(Vector3(500, 500, 500));//test for constrains and solvers
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(5, 5, 3.5f, 3.5f);//create rand cube and sphere
	InitGameExamples();
	InitDefaultFloor();
	BridgeConstraintTest();//!!!s
	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));//state machine code
	testStateObject1 = AddStateObjectToWorld(Vector3(0, 20, 0));//state machine code
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);
	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 20;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(-50, 300, 50);
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);
	GameObject* previous = start;
	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1)* cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}
void TutorialGame::Bridge(Vector3 startpos) {//make this like the real bridge
	Vector3 cubeSize = Vector3(1, 0.2, 3);
	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 25;
	float maxDistance = 2; // constraint distance
	float cubeDistance = 2; // distance between links

	Vector3 startPos = startpos;
	//Vector3 startPos = Vector3(0, 5, 0);
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);
	GameObject* previous = start;
	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize	= Vector3(100, 2, 100);
	AABBVolume* volume	= new AABBVolume(floorSize);
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
GameObject* TutorialGame::AddDeathFloor(const Vector3& position) {
	GameObject* floor = new GameObject("deathfloor");
	Vector3 floorSize = Vector3(500, 2, 500);
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
//add wall to world
GameObject* TutorialGame::AddWallToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(1, 10, 100);
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
}//add wall to world
GameObject* TutorialGame::AddWallRLToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(100, 10, 1);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	//floor->GetPhysicsObject()->set

	world->AddGameObject(floor);

	return floor;
}//add wall to world
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

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

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
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 0.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
	AddWallToWorld(Vector3(100, 10, 0));//wall top
	AddWallToWorld(Vector3(-100, 10, 0));//wall bottom
	AddWallRLToWorld(Vector3(0, 10, 100));//wall right
	AddWallRLToWorld(Vector3(0, 10, -100));//wall left
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
	//AddCapsuleToWorld(Vector3(50, 50, 50), 10.0f, 8.0f);
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}
StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {//state machine code
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}
StateGameObject* TutorialGame::AddStateWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {//state machine code for auto up and down wall
	StateGameObject* apple = new StateGameObject();

	//SphereVolume* volume = new SphereVolume(0.25f);
	AABBVolume* volume = new AABBVolume(dimensions);

	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(dimensions * 2)
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), cubeMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(inverseMass);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
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
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				//add draw information function here
				float x = selectionObject->GetTransform().GetPosition().x;
				float y = selectionObject->GetTransform().GetPosition().y;
				float z = selectionObject->GetTransform().GetPosition().z;
				string pos = std::to_string(x);
				pos = pos + ',';
				pos += std::to_string(y);
				pos = pos + ',';
				pos += std::to_string(z);
				renderer->DrawString(pos, Vector2(10, 10));
				string name;
				if (selectionObject->GetName() == "") {
					name = "noName";
					renderer->DrawString(name, Vector2(10, 15));
				}
				else {
					name = selectionObject->GetName();
					renderer->DrawString(name, Vector2(10, 15));
				}
				Quaternion tempnum;
				tempnum=selectionObject->GetTransform().GetOrientation();
				//float x, y, z, w;
				string sx, sy, sz, sw, all;
				float ox = tempnum.x;
				float oy = tempnum.y;
				float oz = tempnum.z;
				float ow = tempnum.w;
				sx = std::to_string(ox);
				sy = std::to_string(oy);
				sz = std::to_string(oz);
				sw = std::to_string(ow);
				//all = sx;
				all = "Orit (" + sx + ',' + sy + ',' + sz + ',' + sw + ')';
				renderer->DrawString(all, Vector2(10, 25));

				//add draw information function here
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

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
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	if (!selectionObject) {
		return;
	}
	isSelected = true;
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				//selectionObject->GetPhysicsObject()->AddForce(ray.GetDirection() * forceMagnitude);//tu1
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
	//use WASD to move selectedobject code begin
	if (!selectionObject) {
		return;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::TAB))
	{
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * forceMagnitude*0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::Q)) {//lets rotate
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::E)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::R)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::J)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 0, 1) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::K)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 0, -1) * forceMagnitude * 0.02f);
		}
		
	}
	//use WASD to move selectedobject code end
}

//coursework function begin
void TutorialGame::DrawMenu() {
	glClearColor(0, 0, 0, 1);
	Debug::FlushRenderables(0);
	renderer->DrawString("Welcome to game", Vector2(10, 10));
	renderer->DrawString("Press '1' for game", Vector2(10, 20));
	renderer->DrawString("Press '2' for AI game", Vector2(10, 30));
	renderer->DrawString("Press '3' for xx game", Vector2(10, 40));
	renderer->Render();
	world->ClearAndErase();
	//enemies.clear();
	//obstacles.clear();
	player = nullptr;
	physics->Clear();
	winnerName.clear();
}
void TutorialGame::DrawWin() {
	renderer->DrawString("you win", Vector2(10, 10));
}
void TutorialGame::DrawLose(std::string winnner) {
	renderer->DrawString("you lose", Vector2(10, 10));
}
void TutorialGame::DrawPause() {
	renderer->DrawString("now game paused", Vector2(10, 10));
}
void TutorialGame::InitGameWorld1() {//ball
	InitialiseAssets();//add assets first
	physics->SetNum();
	world->ClearAndErase();
	physics->Clear();
	pathfound = false;

	//InitGameExamples();
	InitDefaultFloor();
	//BridgeConstraintTest();//!!!s
	
	AddCubeToWorld(Vector3(-80, 0, 0), Vector3(3, 9, 3), 0.0f);
	//AddCapsuleToWorld(Vector3(-85, 3, 0), 9.0f,5.0f, 6.0f);
	AddCubeToWorld(Vector3(-74, 0, 0), Vector3(3, 6, 3), 0.0f);
	AddCubeToWorld(Vector3(-68, 0, 0), Vector3(3, 3, 3), 0.0f);
	AddSpherePlayerToWorld(Vector3(-76, 16, 0), 2.5, 6.0f);//add player

	AddJumpPad(Vector3(-56, 0, 0), Vector3(9, 1, 3), 0.0f);//jump pad
	testStateObject = AddStateWallToWorld(Vector3(-44,0,-8), Vector3(3,3,3), 10.0f);//state machine code
	AddJumpPad(Vector3(-32, 0, 0), Vector3(9, 1, 3), 0.0f);//jump pad
	testStateObject1 = AddStateWallToWorld(Vector3(-20, 0, -8), Vector3(3, 3, 3), 10.0f);//state machine code
	AddIcePad(Vector3(3, 0, 0), Vector3(15, 6, 3), 0.0f);
	Bridge(Vector3(23,5,0));
	AddEndPad(Vector3(100, 10, 0), Vector3(3, 20, 20), 0.0f);
	AddInclinePad(Vector3(-56, 0, -6), Vector3(9, 1, 3), Quaternion(1, 0, 0, 3), 0.0f);
	AddCoin(Vector3(-44, 16, 0), 0.25f, 0.0f);
	AddCoin(Vector3(-20, 16, 0), 0.25f, 0.0f);
	AddCoin(Vector3(3, 8, 0), 0.25f, 0.0f);
	AddCoin(Vector3(23, 8, 0), 0.25f, 0.0f);//on bridge
	AddCoin(Vector3(38, 8, 0), 0.25f, 0.0f);//on bridge
	AddCoin(Vector3(53, 8, 0), 0.25f, 0.0f);//on bridge
	AddCoin(Vector3(68, 8, 0), 0.25f, 0.0f);//on bridge
	AddDeathFloor(Vector3(0, -50, 0));
	AddCubeToWorld(Vector3(3, 0, 15), Vector3(3, 7, 3), 0.0f);
	AddCubeToWorld(Vector3(3, 0, -15), Vector3(3, 7, 3), 0.0f);
	AddSpin(Vector3(3, 9, 15), Vector3(1, 2, 14.5), Quaternion(0, 1, 0, 1), 10.0f,"spinright");
	AddSpin(Vector3(3, 9, -15), Vector3(1, 2, 14.5), Quaternion(0, 1, 0, 1), 10.0f,"spinleft");
	//AddInclinePad(Vector3(-30, 0, -30), Vector3(20, 20, 20), Quaternion(1, 0, 0, 3), 0.0f);
	AddCapsuleToWorld(Vector3(60, 20, 60), 6.0f, 5.0f, 5.0f);
	AddCapsuleToWorld(Vector3(70, 30, 70), 9.0f, 5.0f, 5.0f);
}
void TutorialGame::InitGameWorld2() {//maze
	InitialiseAssets();//add assets first
	world->ClearAndErase();
	physics->Clear();
	physics->SetNum();
	pathfound = false;

	AddFloorToWorld(Vector3(0, 0, 0));
	InitMap();
	AddPlayerToWorld(Vector3(35, 2, 50));
	AddEnemyToWorld(Vector3(35, 2, -50));
	PathFinding();
	
	testStateObject = AddStateObjectToWorld(Vector3(-101, 10, -101));//state machine code,before delete change updategame func
	testStateObject1 = AddStateObjectToWorld(Vector3(-101, 20, -101));//state machine code,before delete change updategame func
}
GameObject* TutorialGame::AddJumpPad(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("jumppad");

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
GameObject* TutorialGame::AddIcePad(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("icepad");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);
	

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));
	
	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	//cube->GetPhysicsObject()->SetElasticity(1);
	cube->GetPhysicsObject()->SetFriction(0.8);

	world->AddGameObject(cube);

	return cube;
}
GameObject* TutorialGame::AddEndPad(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("Endpad");

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
GameObject* TutorialGame::AddSpherePlayerToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphereplayer");

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
GameObject* TutorialGame::AddInclinePad(const Vector3& position, Vector3 dimensions, Quaternion qutn, float inverseMass) {
	GameObject* cube = new GameObject("Inclinepad");

	//AABBVolume* volume = new AABBVolume(dimensions);
	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);
	cube->GetTransform().SetOrientation(qutn);//set Quaternion

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetFriction(10);


	world->AddGameObject(cube);

	return cube;
}
GameObject* TutorialGame::AddCoin(const Vector3& position, float radius, float inverseMass) {
	GameObject* coin = new GameObject("coin");

	SphereVolume* volume = new SphereVolume(radius);
	coin->SetBoundingVolume((CollisionVolume*)volume);
	coin->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	coin->SetRenderObject(new RenderObject(&coin->GetTransform(), bonusMesh, nullptr, basicShader));
	coin->SetPhysicsObject(new PhysicsObject(&coin->GetTransform(), coin->GetBoundingVolume()));

	coin->GetPhysicsObject()->SetInverseMass(inverseMass);
	coin->GetPhysicsObject()->InitSphereInertia();
	coin->GetPhysicsObject()->SetElasticity(0);
	coin->GetPhysicsObject()->SetFriction(0);


	world->AddGameObject(coin);

	return coin;
}
GameObject* TutorialGame::AddSpin(const Vector3& position, Vector3 dimensions, Quaternion qutn, float inverseMass,string name) {
	GameObject* cube = new GameObject(name);
	AABBVolume* volume = new AABBVolume(dimensions);
	//OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions*2);
	//cube->GetTransform().SetOrientation(qutn);//set Quaternion

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
void TutorialGame::BuildCubeWall(float xAxisNum,float zAxisNum, Vector3 startpos, int cubenum, Vector3 cubeDimension, float inverseMass) {
	//build cube wall in maze with easy way
	float dx = cubeDimension.x;
	float dz = cubeDimension.z;
	for (int x = 0; x < xAxisNum; ++x) {
		for (int z = 0; z < zAxisNum; ++z) {
			startpos = Vector3(startpos.x*x*dx, 0.0f, startpos.z*z*dz);
			AddCubeToWorld(startpos, cubeDimension,inverseMass);
		}
	}
}
void TutorialGame::InitMap() {
	map = new NavigationGrid("Map1.txt");
	pathfound = false;
	//map = new NavigationGrid("TestGrid1.txt");

	GridNode* nodes = map->GetNodes();
	
	float x, y, z;
	Vector3 pos;
	int nodesize = map->GetNodesize();
	int width = map->GetWidth();
	int height = map->GetHeight();
	for (int z = 0; z < height; ++z) {
		for (int x = 0; x < width; ++x) {
			GridNode& n = nodes[(width * z) + x];
			if (n.type == 'x') {
				pos.x = n.position.x - 45;
				pos.y = 7;
				pos.z = n.position.z - 45;
				AddCubeToWorld(pos,Vector3(0.5,0.5,0.5)*nodesize,0.0f);
				//AddCubeToWorld(n.position, Vector3(0.5, 0.5, 0.5) * nodesize, 0.0f);
			}
			//if (n.type == '.') {
			//	AddCubeToWorld(n.position, Vector3(0.5, 0.1, 0.5) * nodesize, 0.0f);
			//}
		}
	}
}
void TutorialGame::PathFinding() {
	NavigationGrid grid("Map1.txt");
	//NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;
	//Vector3 startPos(35, 2, 45);
	//Vector3 endPos(35, 2, -45);
	//Vector3 startPos(80, 0, 10);//tu
	//Vector3 endPos(80, 0, 80);//tu
	Vector3 startPos(80, 0, 0);
	Vector3 endPos(80, 0, 90);

	//bool found = grid.FindPath(playerpos, enemypos, outPath);//use player pos and enemy pos to update path
	bool found = grid.FindPath(startPos, endPos, outPath);
	if (found) {
		std::cout << "path found\n";
		pathfound = true;
	}
	else {
		std::cout << "no path\n";
		pathfound = false;
	}
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos+Vector3(0,3,0));
	}
	
}
void TutorialGame::DisplayPathfinding() {//path finding
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		//Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));//change this startpos.x and y same on endpos//
		Debug::DrawLine(Vector3(a.x-45.0f,a.y,a.z-45.0f), Vector3(b.x-45.0f,b.y,b.z-45.0f), Vector4(0, 1, 0, 1));//change this startpos.x and y same on endpos//
	}
}
//coursework function end