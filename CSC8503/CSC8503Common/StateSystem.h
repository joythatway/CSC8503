#pragma once
#include <vector>
#include"../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/PushdownMachine.h"

namespace NCL
{
	namespace CSC8503
	{
		class StateSystem
		{
		public:
			StateSystem();
			~StateSystem();

			void Update(float dt);

			void AddMachine(StateMachine* sm);
			void AddMachine(PushdownMachine* pdm);

			void Clear()
			{
				for (auto& i : machines_pushdown) {
					delete i;
				}
				for (auto& i : machines_state) {
					delete i;
				}

				machines_pushdown.clear();
				machines_state.clear();
			}
		protected:
			std::vector<StateMachine*>		machines_state;
			std::vector<PushdownMachine*>	machines_pushdown;
		};
	}
}