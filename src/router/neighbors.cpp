/**
*	neighbors.cpp - Часть модуля маршрутизации в котором
*	хранятся основные публичные функции модуля.
*
*	@mrrva - 2019
*/
#include "../include/router.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_router::s_neighbors - Функция обработки полученных
*	соседних клиентов.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
struct tin_task _router::s_neighbors(tinmsg &msg,
	struct sockaddr_in &skddr)
{
	using tinstorage::neighbors;
	using tinstorage::nodes;

	vector<unsigned char *> list;
	struct tin_ipport ipport;
	struct tin_task task;
	struct tin_node node;

	list = msg.info_neighbors();
	ipport = ipport_get(skddr);

	if (!list.empty()) {
		task.bytes[0] = 0x00;

		if (!nodes.find_ip(node, ipport.ip))
			return task;

		neighbors.clear(node.hash);

		for (auto &p : list) {
			neighbors.add(node.hash, p);
			delete[] p;
		}

		return task;
	}

	return this->neighbors_list(skddr);
}
/**
*	_router::neighbors_list - Функция создания списка соседей
*	для отправки.
*
*	@s - Структура sockaddr_in.
*/
struct tin_task _router::neighbors_list(struct sockaddr_in &s)
{
	using tinstruct::clients;

	size_t prt = INFOSIZE / HASHSIZE, n;
	struct tin_task task;
	unsigned char *buff;

	prt = (prt > clients.size()) ? clients.size() : prt;
	buff = msg_tmp<true>(S_REQUEST_CLIENTS);

	for (size_t i = 0; i < prt; i++) {
		n = HASHSIZE + 1 + HASHSIZE * i++;

		if (n >= HEADERSIZE)
			break;

		memcpy(buff + n, clients[i].hash, HASHSIZE);
	}

	memcpy(task.bytes, buff, HEADERSIZE);
	task.length = HEADERSIZE;
	task.target_only = true;
	task.client_in = s;

	delete[] buff;
	return task;
}
