#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/StateGameObject.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/State.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			//coursework function begin
			void DrawMenu();
			void DrawWin();
			void DrawLose(std::string winner);
			void DrawPause();
			StateMachine* gameState;

			//coursework function end

		protected:
			//coursework begin
			PushdownMachine* machine;
			std::string winnerName;
			GameObject* player = nullptr;
			bool multiplayer;
			bool useGravity;
			bool inSelectionMode;
			bool inMainMenu;
			bool paused;
			bool withFriends;
			bool finished;
			//coursework end
			// 
			//state machine begin
			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject = nullptr;
			StateGameObject* testStateObject1 = nullptr;//
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

			GameTechRenderer* renderer;
			PhysicsSystem* physics;
			GameWorld* world;

			//bool useGravity;
			//bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh* capsuleMesh = nullptr;
			OGLMesh* cubeMesh = nullptr;
			OGLMesh* sphereMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLShader* basicShader = nullptr;

			//Coursework Meshes
			OGLMesh* charMeshA = nullptr;
			OGLMesh* charMeshB = nullptr;
			OGLMesh* enemyMesh = nullptr;
			OGLMesh* bonusMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
			//pushdown begin
			class PauseScreen : public PushdownState
			{
			public:
				PushdownResult OnUpdate(float dt, PushdownState** newState) override
				{
					if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U))
					{
						return PushdownResult::Pop;
					}
					return PushdownResult::NoChange;
				}

				void OnAwake() override
				{
					std::cout << "Press U to unpause game!" << std::endl;
				}
			};

			class GameScreen : public PushdownState
			{
			public:
				PushdownResult OnUpdate(float dt, PushdownState** newState) override
				{
					pauseReminder -= dt;

					if (pauseReminder < 0)
					{
						std::cout << "Coins mined: " << coinsMined << std::endl;
						std::cout << "Press P to pause game, or F1 to return to the main menu!" << std::endl;

						pauseReminder += 1.0f;
					}
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P))
					{
						*newState = new PauseScreen();
						return PushdownResult::Push;
					}
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1))
					{
						std::cout << "Returning to main menu!" << std::endl;
						return PushdownResult::Pop;
					}
					if (rand() % 7 == 0)
					{
						coinsMined++;
					}
					return PushdownResult::NoChange;
				}

				void OnAwake() override
				{
					std::cout << "Preparing to mine coins!" << std::endl;
				}

			protected:
				int coinsMined = 0;
				float pauseReminder = 1.0f;
			};

			class IntroScreen : public PushdownState
			{
			public:
				PushdownResult OnUpdate(float dt, PushdownState** newState) override
				{
					if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE))
					{
						*newState = new GameScreen();
						return PushdownResult::Push;
					}
					if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE))
					{
						return PushdownResult::Pop;
					}
					return PushdownResult::NoChange;
				}

				void OnAwake() override
				{
					std::cout << "Welcome to a really awesome game" << std::endl;
					std::cout << "Press space to begin or escape to quit" << std::endl;
				}
			};
			//pushdown end
		};
	}
}

