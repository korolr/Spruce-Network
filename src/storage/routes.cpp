/**
*	routes.cpp - Модуль отвечающий за работу с
*	структурой DNS записей сети TGN.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_routes::add - Функция добавления записи в
*	общий список DNS записей.
*
*	@hash - Хэш клиента.
*	@ipport - Структура ip & port.
*	@find - Ищем ли мы участника маршрута.
*/
void _routes::add(unsigned char *hash,
	struct tgn_ipport ipport, bool find)
{
	struct tgn_route record;

	if (!hash || this->exists(hash) > 0)
		return;

	memcpy(record.hash, hash, HASHSIZE);
	record.ping = chrono::system_clock::now();
	record.ipport = ipport;
	record.find = find;

	this->mute.lock();
	tgnstruct::routes.push_back(record);
	this->mute.unlock();
}
/**
*	_routes::exists - Проверка существования записи
*	в общем списке.
*
*	@hash - Хэш клиента.
*/
size_t _routes::exists(unsigned char *hash)
{
	size_t status = 0;

	if (!hash || hash == nullptr)
		return 0;

	this->mute.lock();

	for (auto &p : tgnstruct::routes)
		if (memcmp(hash, p.hash, HASHSIZE) == 0) {
			status = (p.find) ? 1 : 2;
			break;
		}

	this->mute.unlock();
	return status;
}
/**
*	_routes::autoremove - Удаление всех неактивных
*	записей из общего списка.
*/
void _routes::autoremove(void)
{
	using chrono::system_clock;
	using chrono::time_point;
	using tgnstruct::routes;

	vector<struct tgn_route>::iterator it;
	time_point<system_clock> time_now;

	this->mute.lock();

	if (routes.empty()) {
		this->mute.unlock();
		return;
	}

	time_now = system_clock::now();

	for (it = routes.begin(); it != routes.end(); it++) {
		if (time_now - (*it).ping > 25200s)
			routes.erase(it);
	}

	this->mute.unlock();
}
/**
*	_routes::remove_hash - Функция удаления маршрута
*	по хэшу клиента.
*
*	@hash - Хэш клиента.
*/
void _routes::remove_hash(unsigned char *hash)
{
	using tgnstruct::routes;

	vector<struct tgn_route>::iterator it;

	if (!hash || hash == nullptr)
		return;

	this->mute.lock();
	
	for (it = routes.begin(); it != routes.end(); it++)
		if (memcmp((*it).hash, hash, HASHSIZE) == 0) {
			routes.erase(it);
			break;
		}

	this->mute.unlock();
}
/**
*	_routes::find - Функция нахождения записи в
*	общем списке DNS.
*
*	@route - Структура с результатом поиска.
*	@hash - Хэш клиента.
*/
bool _routes::find(struct tgn_route &route,
	unsigned char *hash)
{
	bool status = false;

	if (!hash || hash == nullptr)
		return false;

	this->mute.lock();

	for (auto &p : tgnstruct::routes) {
		if (memcmp(hash, p.hash, HASHSIZE) == 0
			&& p.find == false) {
			p.ping = chrono::system_clock::now();
			status = true;
			route = p;
			break;
		}
	}

	this->mute.unlock();
	return status;
}
/**
*	_routes::update - Обновление ip адреса ноды
*	в DNS записи.
*
*	@hash - Хэш клиента.
*	@ipport - Новая структура данных ipport.
*/
void _routes::update(unsigned char *hash,
	struct tgn_ipport ipport)
{
	if (!hash || ipport.ip.length() < 6
		|| ipport.port == 0)
		return;

	this->mute.lock();

	for (auto &p : tgnstruct::routes)
		if (memcmp(hash, p.hash, HASHSIZE) == 0) {
			p.ping = chrono::system_clock::now();
			p.ipport = ipport;
			p.find = false;
			break;
		}

	this->mute.unlock();
}

