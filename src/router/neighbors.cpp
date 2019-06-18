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
*	_router::req_neighbors - Функция обработки запросов
*	на получение соседних клиентов.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::req_neighbors(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstruct::clients;

	size_t prt = INFOSIZE / HASHSIZE, n;
	size_t hs = HASHSIZE;
	unsigned char *buff;

	prt = (prt > clients.size()) ? clients.size()
		: prt;
	buff = msg_tmp<true>(S_RESPONSE_CLIENTS);

	for (size_t i = 0; i < prt; i++) {
		n = hs + 1 + hs * i++;

		if (n >= HEADERSIZE)
			break;

		memcpy(buff + n, clients[i].hash, hs);
	}

	return buff;
}
/**
*	_router::rsp_neighbors - Функция обработки полученных
*	соседей других нод.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::rsp_neighbors(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::neighbors;
	using tgnstorage::nodes;

	vector<unsigned char *> list;
	struct tgn_ipport ipport;
	struct tgn_node node;

	list = msg.info_neighbors();
	ipport = ipport_get(skddr);

	if (!nodes.find_ip(node, ipport.ip)
		|| list.empty()) {
		return nullptr;
	}

	neighbors.clear(node.hash);

	for (auto &p : list) {
		neighbors.add(node.hash, p);
		delete[] p;
	}

	return nullptr;
}
