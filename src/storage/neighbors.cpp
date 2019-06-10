/**
*	neighbors.cpp - Модуль отвечающий за работу с
*	структурой соседних клиентов сети TGN.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_neighbors::add - Функция добавление нового соседа
*	в общий список.
*
*	@node - Хэш ноды клиента.
*	@client - Хэш клиента.
*/
void _neighbors::add(unsigned char *node,
	unsigned char *client) {

	struct tgn_neighbor neighbor;

	if (!node || !client || this->exists(client))
		return;

	this->mute.lock();

	neighbor.ping = chrono::system_clock::now();
	memcpy(neighbor.client, client, HASHSIZE);
	memcpy(neighbor.node, node, HASHSIZE);

	tgnstruct::neighbors.push_back(neighbor);
	this->mute.unlock();
}
/**
*	_neighbors::exists - Проверка существования клиента
*	в общем списке.
*
*	@client - Хэш клиента.
*/
bool _neighbors::exists(unsigned char *client)
{
	bool status = false;

	if (!client || client == nullptr)
		return false;

	this->mute.lock();

	for (auto &p : tgnstruct::neighbors) {
		if (memcmp(p.client, client, HASHSIZE) == 0) {
			status = true;
			break;
		}
	}

	this->mute.unlock();
	return status;
}
/**
*	_neighbors::clear - Очистка всех клиентов по
*	определенной ноде.
*
*	@hash - Хэш ноды.
*/
void _neighbors::clear(unsigned char *hash)
{
	using tgnstruct::neighbors;

	vector<struct tgn_neighbor>::iterator it;

	if (!hash || hash == nullptr)
		return;

	this->mute.lock();
	it = neighbors.begin();

	for (; it != neighbors.end(); it++) {
		if (memcmp((*it).node, hash, HASHSIZE) == 0)
			neighbors.erase(it);
	}

	this->mute.unlock();
}
/**
*	_neighbors::find - Поиск клиента в общем списке.
*
*	@buff - Буфер выгрузки данных.
*	@hash - Хэш клиента.
*/
bool _neighbors::find(struct tgn_neighbor &buff,
	unsigned char *hash)
{
	bool status = false;

	if (!hash || hash == nullptr)
		return false;

	this->mute.lock();

	for (auto &p : tgnstruct::neighbors)
		if (memcmp(hash, p.client, HASHSIZE) == 0) {
			p.ping = chrono::system_clock::now();
			status = true;
			buff = p;
			break;
		}

	this->mute.unlock();
	return status;
}
/**
*	_neighbors::timelist - Составления списка последнего
*	получения клиентов от нод.
*/
vector<time_list> _neighbors::timelist(void)
{
	using tgnstruct::neighbors;

	typedef time_point<system_clock> time;
	typedef unsigned char uc;

	vector<time_list> list;
	unsigned char *key;
	bool flag = false;

	for (auto &n : neighbors) {
		for (auto &l : list)
			if (memcmp(n.node, l.first, HASHSIZE) == 0) {
				flag = true;
				break;
			}

		if (!flag)
			continue;

		key = new unsigned char[HASHSIZE];
		memcpy(key, n.node, HASHSIZE);
		list.push_back(pair<uc *, time>(key, n.ping));
	}

	return list;
}
