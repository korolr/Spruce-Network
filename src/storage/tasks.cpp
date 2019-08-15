/**
*	tasks.cpp - The module responsible for work
*	with job structure TGN network.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Used namespaces and objects.
*/
using namespace std;
/**
*	_nodes::add - Add a new task to
* 	general list of tasks.
*
*	@task - Structure of the new task.
*/
void _tasks::add(struct tgn_task task)
{
	this->mute.lock();
	tgnstruct::tasks.push_back(task);
	this->mute.unlock();
}
/**
*	_nodes::remove_first - Delete First
* 	tasks from the general list.
*/
void _tasks::remove_first(void)
{
	using tgnstruct::tasks;

	this->mute.lock();
	tgnstruct::tasks.erase(tasks.begin());
	this->mute.unlock();
}
