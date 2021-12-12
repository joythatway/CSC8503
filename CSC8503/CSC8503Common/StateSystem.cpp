#include "../CSC8503Common/StateSystem.h"

using namespace NCL;
using namespace CSC8503;

StateSystem::StateSystem()
{
}

StateSystem::~StateSystem()
{
	//delete all of the machines?
	for (std::vector<StateMachine*>::iterator i = machines_state.begin(); i != machines_state.end(); i++)
		delete (*i);

	for (std::vector<PushdownMachine*>::iterator i = machines_pushdown.begin(); i != machines_pushdown.end(); i++)
		delete (*i);
}

void StateSystem::Update(float dt)
{
	//Update both vectors
	for (std::vector<StateMachine*>::iterator i = machines_state.begin(); i != machines_state.end(); i++)
		(*i)->Update(dt);

	for (std::vector<PushdownMachine*>::iterator i = machines_pushdown.begin(); i != machines_pushdown.end(); i++)
		(*i)->Update(dt);
}

void StateSystem::AddMachine(StateMachine* sm)
{
	machines_state.emplace_back(sm);
}

void StateSystem::AddMachine(PushdownMachine* sm)
{
	machines_pushdown.emplace_back(sm);
}