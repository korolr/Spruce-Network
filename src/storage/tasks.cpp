/**
*	tasks.cpp - Модуль отвечающий за работу с
*	структурой заданий сети tin.
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
void _tasks::remove(vector<struct tin_task>::iterator i)
{
	using tinstruct::tasks;

	vector<struct tin_task>::iterator p;

	if (tasks.empty())
		return;

	this->mute.lock();
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
void _tasks::add(struct tin_task task)
{
	this->mute.lock();
	tinstruct::tasks.push_back(task);
	this->mute.unlock();
}