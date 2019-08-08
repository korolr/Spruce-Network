/**
*	nodes.cpp - Модуль отвечающий за работу с
*	структурой клиентов сети TGN.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_clients::update - Обновляет и добавляет клиентов
*	сети.
*
*	@hash - Публичный ключ клиента.
*	@ipport - Структура ipport.
*/
void _clients::update(unsigned char *hash,
	struct tgn_ipport &ipport)
{
	struct tgn_client new_one;

	this->mute.lock();

	for (auto &p : tgnstruct::clients)
		if (p.ipport.ip == ipport.ip) {
			p.ping = chrono::system_clock::now();
			memcpy(p.hash, hash, HASHSIZE);
			this->mute.unlock();
			return;
		}

	new_one.ping = chrono::system_clock::now();
	memcpy(new_one.hash, hash, HASHSIZE);
	new_one.ipport = ipport;

	tgnstruct::clients.push_back(new_one);
	this->mute.unlock();
}
/**
*	_clients::autoremove - Удаление пользователей,
*	которые не посылали запросы дольше 6 секунд.
*/
void _clients::autoremove(void)
{
	using tgnstruct::clients;

	time_point<system_clock> time_now;

	this->mute.lock();

	if (clients.empty()) {
		this->mute.unlock();
		return;
	}

	time_now = chrono::system_clock::now();

	for (size_t i = 0; i < clients.size(); i++)
		if (time_now - clients[i].ping > 6s) 
			clients.erase(clients.begin() + i);
	this->mute.unlock();
}
/**
*	_clients::exists - Проверка существования клиента
*	в общем списке.
*
*	@hash - Хэш клиента.
*/
bool _clients::exists(unsigned char *hash)
{
	bool status = false;

	if (!hash || hash == nullptr)
		return false;

	this->mute.lock();

	for (auto &p : tgnstruct::clients)
		if (memcmp(hash, p.hash, HASHSIZE) == 0) {
			status = true;
			break;
		}

	this->mute.unlock();
	return status;
}
/**
*	_clients::find - Функция нахождения клиента в
*	общем списке клиентов.
*
*	@buffer - Структура записи клиента.
*	@hash - Хэш клиента.
*/
bool _clients::find(struct tgn_client &buffer,
	unsigned char *hash)
{
	bool status = false;

	if (!hash || hash == nullptr)
		return false;

	this->mute.lock();

	for (auto &p : tgnstruct::clients)
		if (memcmp(hash, p.hash, HASHSIZE) == 0) {
			status = true;
			buffer = p;
			break;
		}

	this->mute.unlock();
	return status;
}

