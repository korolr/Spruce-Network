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
/**
*	_nodes::remove_first - Удаление первого
*	задания из общего списка.
*/
void _tasks::remove_first(void)
{
	using tgnstruct::tasks;

	this->mute.lock();
	tgnstruct::tasks.erase(tasks.begin());
	this->mute.unlock();
}