#pragma once
#include <stack>

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			//PushdownMachine();
			PushdownMachine(PushdownState* initialState) {//Pushdown Automata
				this->initialState = initialState;
			}

			~PushdownMachine() {};

			bool Update(float dt);//Pushdown Automata
			//void Update();

		protected:
			PushdownState* activeState;

			PushdownState* initialState;//Pushdown Automata

			std::stack<PushdownState*> stateStack;
		};
	}
}

