#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/StateGameObject.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "..//CSC8503Common/NavigationGrid.h"
#include "..//CSC8503Common/NavigationMap.h"
#include "..//CSC8503Common/NavigationPath.h"


namespace NCL {
	namespace CSC8503 {
		
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();
			virtual void UpdateGame(float dt);
			//coursework function begin
			bool isSelected = false;
			void DrawMenu();
			void DrawWin();
			void DrawLose(std::string winner);
			void DrawPause();
			void InitGameWorld1();//ball
			void InitGameWorld2();//maze

			bool UpdatePushdown(float dt) {
				if (!machine->Update(dt)) {
					return false;
				}
				return true;
			}
			//coursework function end

		protected:
			//coursework begin
			NavigationGrid* map;
			PushdownMachine* machine;
			std::string winnerName;
			GameObject* player = nullptr;
			bool multiplayer = 0;
			bool gameoneortwo = 0;
			void BuildCubeWall(float xAxisNum,float zAxisNum, Vector3 startpos, int cubenum, Vector3 cubeDimension, float inverseMass);
			//coursework end
			// 
			void PathFinding();
			void DisplayPathfinding();
			vector <Vector3 > testNodes;//path finding
			GameObject* AddPlayer1ToWorld(const Vector3& position);
			GameObject* AddEnemy1ToWorld(const Vector3& position);

			GameObject* AddSpherePlayerToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			//state machine begin
			StateGameObject* AddStateWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 0.0f);
			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject = nullptr;
			StateGameObject* testStateObject1 = nullptr;//
			StateGameObject* testStateWall1 = nullptr;//
			//state machine end

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
			void Bridge(Vector3 startpos);//bridge
			GameObject* AddJumpPad(const Vector3& position, Vector3 dimensions, float inverseMass);//jump
			GameObject* AddIcePad(const Vector3& position, Vector3 dimensions, float inverseMass);//speed up
			GameObject* AddEndPad(const Vector3& position, Vector3 dimensions, float inverseMass);//End condition
			GameObject* AddInclinePad(const Vector3& position, Vector3 dimensions, Quaternion qutn,float inverseMass);//
			GameObject* AddCoin(const Vector3& position, float radius, float inverseMass = 10.0f);//add coin to get score
			GameObject* AddDeathFloor(const Vector3& position);
			GameObject* AddSpin(const Vector3& position, Vector3 dimensions, Quaternion qutn, float inverseMass,string name);
			void InitMap();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddWallToWorld(const Vector3& position);//add wall to world
			GameObject* AddWallRLToWorld(const Vector3& position);//add wall to world

			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;
			

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

		//};//tutorialgame class 
		class IntroScreen :public PushdownState {//for intro menu screen 
		protected:
			TutorialGame* tugame;
			bool GameMode = 0;
		public:
			IntroScreen(TutorialGame* tugame) {
				this->tugame = tugame;
			}
			PushdownResult OnUpdate(float dt, PushdownState** newstate) override {
				//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
				//	*newstate = new GameScreen(tugame, 0);
				//	return PushdownResult::Push;
				//}
				if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM1)) {
					*newstate = new GameScreen(tugame, 0);
					GameMode = 0;
					return PushdownResult::Push;
				}
				if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2)) {
					// game mode 2
					*newstate = new GameScreen(tugame, 1);
					GameMode = 1;
					return PushdownResult::Push;
				}
				if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
					return PushdownResult::Pop;
				}

				tugame->DrawMenu();
				//tugame->UpdateGame(0);

				return PushdownState::NoChange;
			}
			void OnAwake() override {
				std::cout << "Press 1 to play single player or 2 for multiplayer. ESC to quit\n";
			}

			void OnSleep() override {
				if (GameMode == 0) {//mode 1 game ball
					tugame->InitGameWorld1();
				}
				if (GameMode == 1) {//mode 2 maze ball
					std::cout << "game mode 2 here " << std::endl;
					tugame->InitGameWorld2();//just call gamemode2 function in here
				}
			}
		};
		/*
		class WinGame :public PushdownState {//for win the game screen 

		};*/
		/*
		class LoseGame :public PushdownState {//for lose game screen 
		protected:
			TutorialGame* tugame;
			std::string winner;
		public:
			LoseGame(TutorialGame* tugame, std::string winner) {
				this->tugame = tugame;
				this->winner = winner;
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override {
				if (Window::GetKeyboard()->KeyDown(KeyboardKeys::M)) {
					tugame->machine->Set(new IntroScreen(tugame));
					
				}

				tugame->DrawLose(winner);

				return PushdownState::NoChange;
			}

			void OnAwake() override {
				std::cout << "Press 1 to play single player or 2 for multiplayer. ESC to quit\n";
			}

			void OnSleep() override {
				tugame->InitialiseAssets();
			}
		};
		*///LoseScreen class
		class GameScreen :public PushdownState {
		protected:
			TutorialGame* tugame;
			float pausesave = 1;
			bool multiplayer;
		public:
			GameScreen(TutorialGame* tugame, bool multiplayer = 0) {
				tugame->multiplayer = multiplayer;
				this->tugame = tugame;
			}
			PushdownResult OnUpdate(float dt, PushdownState** newstate) override {
				/*if (!tugame->winnerName.empty()) {
					std::cout << "win\n";
					*newstate = new LoseGame(tugame, tugame->winnerName);
					return PushdownResult::Push;
				}*/

				if (pausesave < 0) {
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::M)) {
						return PushdownResult::Pop;
					}
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
						*newstate = new PauseScreen(tugame);
						return PushdownResult::Push;
					}
				}
				else {
					pausesave -= dt;
				}
				tugame->UpdateGame(dt);

				return PushdownResult::NoChange;
			}
			void OnAwake() override {
				std::cout << "Resuming Game\n";
				pausesave = 0.2;
			}
		};
		class PauseScreen :public PushdownState {//for player press pause buttom screen 
		protected:
			TutorialGame* tugame;
			float pausesave = 1;
		public:
			PauseScreen(TutorialGame* tugame) {
				this->tugame = tugame;
			}
			PushdownResult OnUpdate(float dt, PushdownState** newstate) override {
				if (pausesave < 0) {
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
						return PushdownResult::Pop;
					}
				}
				else {
					pausesave -= dt;
				}
				tugame->UpdateGame(0);
				//tugame->DrawPause();
				return PushdownResult::NoChange;
			}
			void OnAwake() override{
				std::cout << "press esc to pause game" << std::endl;
				pausesave = 0.2;
			}
		};
		};//tutorialgame class end
	}
}

