/**
*	tasks.cpp - Модуль отвечающий за работу с
*	структурой заданий сети TGN.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_nodes::remove - Удаление конкретного задания
*	из общего списка заданий.
*
*	@i - Итератор задания.
*/
void _tasks::remove(vector<struct tgn_task>::iterator i)
{
	using tgnstruct::tasks;

	vector<struct tgn_task>::iterator p;

	this->mute.lock();

	if (tasks.empty()) {
		this->mute.unlock();
		return;
	}

	p = tasks.begin();

	for (; p != tasks.end(); p++)
		if (p == i) {
			tasks.erase(p);
			break;
		}

	this->mute.unlock();
}
/**
*	_nodes::add - Добавление нового задания в
*	общий список заданий.
*
*	@task - Структура нового задания.
*/
void _tasks::add(struct tgn_task task)
{
	this->mute.lock();
	tgnstruct::tasks.push_back(task);
	this->mute.unlock();
}

void _tasks::remove_first(void)
{
	using tgnstruct::tasks;

	this->mute.lock();
	tgnstruct::tasks.erase(tasks.begin());
	this->mute.unlock();
}