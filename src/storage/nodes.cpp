/**
*	nodes.cpp - Модуль отвечающий за работу с
*	структурой нод сети TGN.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_nodes::find_ip - Нахождение ноды сети по ip
*	адресу.
*
*	@f_node - Ссылка для записи адреса ноды.
*	@ip - Ip адрес искомой ноды.
*/
bool _nodes::find_ip(struct tgn_node &f_node,
	string ip)
{
	bool status = false;

	this->mute.lock();

	for (auto &p : tgnstruct::nodes)
		if (p.ip == ip) {
			f_node = p;
			status = true;
			break;
		}

	this->mute.unlock();
	return status;
}
/**
*	_nodes::find_hash - Нахождение ноды сети по hash
*	ключу.
*
*	@f_node - Ссылка для записи адреса ноды.
*	@hash - Hash искомой ноды.
*/
bool _nodes::find_hash(struct tgn_node &f_node,
	unsigned char *hash)
{
	bool status = false;

	this->mute.lock();

	for (auto &p : tgnstruct::nodes)
		if (memcmp(p.hash, hash, HASHSIZE)) {
			f_node = p;
			status = true;
			break;
		}

	this->mute.unlock();
	return status;
}
/**
*	_nodes::ping - Обновление время использования
*	ноды сети.
*
*	@skddr - Структура sockaddr_in ноды.
*/
void _nodes::ping(struct sockaddr_in &skddr)
{
	struct tgn_ipport ipport = ipport_get(skddr);
	struct tgn_node node;

	if (ipport.ip.length() < 5 || 
		ipport.port != PORT)
		return;

	if (!this->find_ip(node, ipport.ip))
		return;

	this->mute.lock();

	node.ping = chrono::system_clock::now();
	tgnstorage::db.ping_node(ipport.ip);

	this->mute.unlock();
}
/**
*	_nodes::remove - Удаление ноды из структуры
*	и базы данных.
*
*	@ip - Ip адрес ноды.
*/
void _nodes::remove(string ip)
{
	using tgnstruct::nodes;

	vector<struct tgn_node>::iterator it;

	this->mute.lock();

	if (ip.length() < 5 || nodes.empty()) {
		this->mute.unlock();
		return;
	}

	it = nodes.begin();

	for (; it != nodes.end(); it++)
		if ((*it).ip == ip) {
			tgnstorage::db.remove_node((*it).ip);
			nodes.erase(it);
			break;
		}

	this->mute.unlock();
}
/**
*	_nodes::add - Добавление новой ноды в общий список
*	и в базу данных.
*
*	@node - Структура данных ноды.
*/
bool _nodes::add(struct tgn_node node)
{
	using tgnstruct::nodes;

	string pub_key = bin2hex<HASHSIZE>(node.hash);
	vector<struct tgn_node>::iterator it;

	this->mute.lock();
	it = nodes.begin();

	for (; it != nodes.end(); it++)
		if ((*it).ip == node.ip) {
			this->mute.unlock();
			return false;
		}

	tgnstorage::db.new_node(node.ip, pub_key);
	node.ping = chrono::system_clock::now();
	nodes.push_back(node);

	this->mute.unlock();
	return true;
}
/**
*	_nodes::select - Извлечение всех нод из базы данных
*	и заполнение списка нод.
*/
void _nodes::select(void)
{
	const short len = HASHSIZE * 2;
	map<string, string> list;
	struct tgn_node node;
	unsigned char *hash;

	list = tgnstorage::db.select_nodes();

	if (list.empty())
		return;

	this->mute.lock();

	for (auto &p : list) {
		node.ping = chrono::system_clock::now();
		hash = hex2bin<len>(p.second);
		memcpy(node.hash, hash, HASHSIZE);
		node.ip = p.first;

		if (hash != nullptr) {
			tgnstruct::nodes.push_back(node);
			delete[] hash;
		}
	}

	this->mute.unlock();
}
/**
*	_nodes::get_last - Получение ноды которую дольше
*	всего не использовали.
*/
struct tgn_node _nodes::get_last(void)
{
	auto last_t = chrono::system_clock::now();
	struct tgn_node node;

	for (auto &p : tgnstruct::nodes)
		if (last_t > p.ping) {
			last_t = p.ping;
			node = p;
		}

	return node;
}
/**
*	_neighbors::autocheck - Автоматическая проверка
*	актуальности списка нод.
*/
void _nodes::autocheck(void)
{
	using tgnstorage::nodes;

	time_point<system_clock> clock;

	this->mute.lock();

	if (tgnstruct::nodes.size() == 0) {
		this->mute.unlock();
		return;
	}

	clock = system_clock::now();

	if (tgnstruct::nodes.size() > MIN_NODES) {
		for (auto &p : tgnstruct::nodes) {
			if (tgnstruct::nodes.size() < 5)
				break;

			if (clock - p.ping > 43200s) {
				this->mute.unlock();
				nodes.remove(p.ip);
				return;
			}
		}

		this->mute.unlock();
		return;
	}

	if (clock - this->last_req < 80s) {
		this->mute.unlock();
		return;
	}

	this->request_getlist();
	this->mute.unlock();
}
/**
*	_neighbors::request_getlist - Создание запроса
*	на получение новых нод.
*/
void _nodes::request_getlist(void)
{
	unsigned char *buffer;
	struct tgn_task task;
	struct tgn_node lst;

	this->last_req = system_clock::now();

	buffer = msg_tmp<true>(S_REQUEST_NODES);
	memcpy(task.bytes, buffer, HEADERSIZE);
	this->last_req = system_clock::now();
	lst = this->get_last();

	task.client_in = saddr_get(lst.ip, PORT);
	task.target_only = false;
	task.length = HEADERSIZE;

	tgnstorage::tasks.add(task);

	delete[] buffer;
}
/**
*	_neighbors::_nodes - Устанавливает время последнего
*	запроса на получения списка нод.
*/
_nodes::_nodes(void)
{
	this->last_req = system_clock::now();
}