#pragma once
#include "../CSC8503Common/GameWorld.h"
#include <set>

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem	{
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				applyGravity = state;
			}

			void SetGlobalDamping(float d) {
				globalDamping = d;
			}

			void SetGravity(const Vector3& g);

			void SetNum();
			void Setenemycatch();
			bool Getcatchflag();
			bool Getenemycatch() {
				return ecatch;
			}
			void Setenemycatchtrue() {
				ecatch = true;
			}
			void Setenemycatchfalse() {
				ecatch = false;
			}
			
		protected:
			bool ecatch = false;
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);

			void UpdateCollisionList();
			void UpdateObjectAABBs();

			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;
			void jumppad(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void icepad(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void Endpad(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void Spinpad(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void penaltyresolvecollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p)const;
			

			GameWorld& gameWorld;

			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo> broadphaseCollisions;//tutorial spatial acceleration structures

			bool useBroadPhase		= true;
			int numCollisionFrames	= 5;
			
		};
	}
}

