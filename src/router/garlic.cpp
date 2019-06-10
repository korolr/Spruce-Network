/**
*	garlic.cpp - Часть модуля маршрутизации в котором
*	хранятся функции обработки чесночных сообщений.
*
*	@mrrva - 2019
*/
#include "../include/router.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_router::u_garlic - Функция обработки чесночных
*	маршрутов клиента.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
struct tgn_task _router::u_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	pair<unsigned char *, unsigned char *> req;
	struct tgn_route route;
	struct tgn_task task;
	unsigned char *hash;

	req = msg.info_garlic();
	hash = msg.byte_key();

	if (!hash || req.first == nullptr || !req.first
		|| bytes_sum<HASHSIZE>(req.first) == 0x00
		|| memcmp(hash, req.first, HASHSIZE) == 0) {
		task.bytes[0] = 0x00;
		goto exit_u_garlic;
	}

	if (tgnstorage::routes.exists(req.first) != 1) {
		this->from_neighbors(req.first);
		this->from_clients(req.first);
	}

	if (tgnstorage::routes.find(route, req.first)) {
		task = this->send_message(route.ipport, msg);
		goto exit_u_garlic;
	}

	this->make_find(req.first);
	task.bytes[0] = 0x00;

exit_u_garlic:
	if (req.second && req.second != nullptr)
		delete[] req.second;
	if (req.first && req.first != nullptr)
		delete[] req.first;
	if (hash && hash != nullptr)
		delete[] hash;

	return task;
}
/**
*	_router::make_find - Функция создания запросов к
*	нодам для поиска клиента.
*
*	@hash - Хэш искомого клиента.
*/
void _router::make_find(unsigned char *hash)
{
	struct tgn_task task;
	struct tgn_ipport ip;
	unsigned char *msg_b;

	if (!hash || hash == nullptr)
		return;

	msg_b = msg_tmp<true>(S_REQUEST_FIND);
	msg_b[HASHSIZE + 1] = 0x00;

	memcpy(msg_b + HASHSIZE + 2, hash, HASHSIZE);
	memcpy(task.bytes, msg_b, HEADERSIZE);

	tgnstorage::routes.add(hash, ip, true);

	for (auto &p : tgnstruct::nodes) {
		task.client_in = saddr_get(p.ip, PORT);
		task.length = HEADERSIZE;
		task.target_only = true;

		tgnstorage::tasks.add(task);
	}

	delete[] msg_b;
}
/**
*	_router::send_message - Функция отправки сообщения
*	через ноду клиенту.
*
*	@ipp - Структура данных ip & port.
*	@msg - Структура сообщения.
*/
struct tgn_task _router::send_message(struct tgn_ipport ipp,
	tgnmsg &msg)
{
	unsigned char *txt, *msg_b;
	struct tgn_task task;

	task.client_in = saddr_get(ipp.ip, ipp.port);
	task.target_only = true;
	task.length = FULLSIZE;

	msg_b = msg_tmp<false>(S_REQUEST_GARLIC);
	txt = msg.garlic_msg();

	memcpy(msg_b + HEADERSIZE, txt, TEXTSIZE);
	memcpy(task.bytes, msg_b, FULLSIZE);

	delete[] msg_b;
	delete[] txt;
	return task;
}
/**
*	_router::from_neighbors - Созданиен маршрута до целели
*	из списка соседних клиентов.
*
*	@hash - Хэш искомого клиента.
*/
void _router::from_neighbors(unsigned char *hash)
{
	using tgnstorage::neighbors;
	using tgnstorage::nodes;

	struct tgn_neighbor neighbor;
	struct tgn_ipport ipport;
	struct tgn_node node;

	if (!hash || !neighbors.find(neighbor, hash)
		|| !nodes.find_hash(node, neighbor.node))
		return;

	ipport = {node.ip, PORT};
	tgnstorage::routes.add(hash, ipport, false);
}
/**
*	_router::from_clients - Определение целели для
*	передачи данных в списке клиентов.
*
*	@hash - Хэш искомого клиента.
*/
void _router::from_clients(unsigned char *hash)
{
	using tgnstorage::clients;
	using tgnstorage::routes;

	struct tgn_client client;

	if (!hash || !clients.find(client, hash))
		return;

	routes.add(hash, client.ipport, false);
}
/**
*	_router::u_garlic - Функция обработки чесночных
*	маршрутов ноды.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
struct tgn_task _router::s_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	pair<unsigned char *, unsigned char *> req;
	struct tgn_client client;
	struct tgn_task task;

	req = msg.info_garlic();

	if (!req.first || !req.second) {
		task.bytes[0] = 0x00;
		return task;
	}

	if (tgnstorage::clients.find(client, req.first)) {
		task = this->send_message(client.ipport, msg);
	}

	delete[] req.second;
	delete[] req.first;
	return task;
}