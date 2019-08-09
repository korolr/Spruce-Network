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

	if (!hash || hash == nullptr)
		return;

	this->mute.lock();

	for (size_t i = 0; i < neighbors.size(); i++) {
		if (memcmp(neighbors[i].node, hash, HASHSIZE) == 0)
			neighbors.erase(neighbors.begin() + i);
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

	this->mute.lock();

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

	this->mute.unlock();
	return list;
}
/**
*	_neighbors::new_requests - Составление новых запросов
*	на получение актуального списка соседей.
*/
void _neighbors::new_requests(void)
{
	using tgnstorage::tasks;

	auto clock = system_clock::now();
	unsigned char *buffer;
	struct tgn_task task;

	if (clock - this->last_req < 80s) {
		return;
	}

	this->last_req = system_clock::now();
	this->mute.lock();

	for (auto &p : tgnstruct::nodes) {
		buffer = msg_tmp<true>(S_REQUEST_CLIENTS);
		memcpy(task.bytes, buffer, HEADERSIZE);

		task.client_in = saddr_get(p.ip, PORT);
		task.length = HEADERSIZE;
		task.target_only = true;

		tasks.add(task);
		delete[] buffer;
	}

	this->mute.unlock();
}
/**
*	_neighbors::autocheck - Автоматическая проверка
*	актуальности списка соседних клиентов.
*/
void _neighbors::autocheck(void)
{
	using tgnstorage::nodes;
	using tgnstorage::tasks;

	vector<time_list> list = this->timelist();
	vector<time_list>::iterator it;
	time_point<system_clock> clock;
	unsigned char *buffer;
	struct tgn_node node;
	struct tgn_task task;

	this->mute.lock();

	if (list.empty()) {
		this->mute.unlock();
		this->new_requests();
		return;
	}

	clock = system_clock::now();
	it = list.begin();

	for (; it != list.end(); it++) {
		if (clock - (*it).second < 900s)
			continue;

		buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

		if (!nodes.find_hash(node, (*it).first)) {
			this->clear((*it).first);
			delete[] (*it).first;
			delete[] buffer;
			continue;
		}

		task.client_in = saddr_get(node.ip, PORT);
		memcpy(task.bytes, buffer, HEADERSIZE);
		task.length = HEADERSIZE;
		task.target_only = true;

		tasks.add(task);
		delete[] (*it).first;
		delete[] buffer;
	}

	this->mute.unlock();
}

_neighbors::_neighbors(void)
{
	this->last_req = system_clock::now();
}