#pragma once
#include <functional>

namespace NCL {
	namespace CSC8503 {
		typedef std::function<void(float)>StateUpdateFunction;

		class State		{
		public:
			State() {}
			State(StateUpdateFunction someFunc) {//state machine code
				func = someFunc;
			}
			//virtual ~State() {}

			void Update(float dt) {//state machine code
				if (func != nullptr) {
					func(dt);
				}
			}
			//virtual void Update() = 0; //Pure virtual base class
		protected:
			StateUpdateFunction func;
		};


		/*
		typedef void(*StateFunc)(void*);
		class GenericState : public State		{
		public:
			GenericState(StateFunc someFunc, void* someData) {
				func		= someFunc;
				funcData	= someData;
			}
			virtual void Update() {
				if (funcData != nullptr) {
					func(funcData);
				}
			}
		protected:
			StateFunc func;
			void* funcData;
		};
		*/
	}
}