/**
*	router.cpp - Часть модуля маршрутизации в котором
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
*	_router::req_nodes - Функция обработки маршрутов
*	для отправки списка нод.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*	@client - Сообщение от клиента.
*/
unsigned char *_router::req_nodes(tgnmsg &msg,
	struct sockaddr_in &skddr, bool client)
{
	size_t i = 0, s, hs = HASHSIZE;
	unsigned char *buff, *ip;
	enum tgn_htype htype;

	htype = (client) ? U_RESPONSE_NODES:
		S_RESPONSE_NODES;
	buff = msg_tmp<true>(htype);

	for (auto &p : tgnstruct::nodes) {
		s = hs + 1 + (hs + 4) * i++;

		if (s >= HEADERSIZE)
			break;

		ip = iptobytes(p.ip);

		if (ip == nullptr) {
			tgnstorage::nodes.remove(p.ip);
			break;
		}

		memcpy(buff + s, p.hash, HASHSIZE);
		memcpy(buff + s + HASHSIZE, ip, 4);
		delete[] ip;
	}

	return buff;
}
/**
*	_router::resp_nodes - Функция обработки маршрутов
*	для получения списка нод.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::rsp_nodes(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	map<unsigned char *, string> list;
	struct tgn_node node;

	list = msg.info_nodes();

	for (auto &p : list) {
		node.ip = p.second;
		memcpy(node.hash, p.first, HASHSIZE);
		tgnstorage::nodes.add(node);

		delete[] p.first;
	}

	return nullptr;
}
