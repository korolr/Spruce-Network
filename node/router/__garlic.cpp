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
*	_router::u_req_garlic - Функция обработки чесночных
*	маршрутов клиента.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::u_req_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::routes;

	pair<unsigned char *, unsigned char *> req;
	unsigned char *hash, *resp;
	struct tgn_route route;
	size_t hs = HASHSIZE;

	req = msg.info_garlic();
	hash = msg.byte_key();

	if (!hash || req.first == nullptr || !req.first
		|| bytes_sum<HASHSIZE>(req.first) == 0x00
		|| memcmp(hash, req.first, HASHSIZE) == 0) {
		resp = nullptr;
		goto exit_u_garlic;
	}

	if (routes.exists(req.first) != 1) {
		this->from_neighbors(req.first);
		this->from_clients(req.first);
	}

	if (routes.find(route, req.first)) {
		resp = msg_tmp<true>(U_RESPONSE_GARLIC);
		this->new_msg(route.ipport, msg, hash);
		memset(resp + hs * 3 + 1, 0x01, 1);
		goto exit_u_garlic;
	}

	resp = msg_tmp<true>(U_RESPONSE_GARLIC);
	memset(resp + hs * 3 + 1, 0x04, 1);

	this->make_find(req.first);

exit_u_garlic:
	if (req.second && req.second != nullptr)
		delete[] req.second;
	if (req.first && req.first != nullptr)
		delete[] req.first;
	if (hash && hash != nullptr)
		delete[] hash;

	return resp;
}
/**
*	_router::u_garlic - Функция обработки чесночных
*	маршрутов ноды.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::s_req_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::clients;

	pair<unsigned char *, unsigned char *> req;
	unsigned char *resp, *info;
	struct tgn_client client;
	size_t hs = HASHSIZE;

	req = msg.info_garlic();
	info = msg.get_info();

	if (!req.first || !req.second) {
		if (req.first) {
			delete[] req.first;
		}
		if (req.second) {
			delete[] req.second;
		}
		return nullptr;
	}

	if (clients.find(client, req.first)) {
		this->new_msg(client.ipport, msg, nullptr);
	}

	resp = msg_tmp<true>(S_RESPONSE_GARLIC);
	memcpy(resp + hs + 1, info, INFOSIZE);

	delete[] req.second;
	delete[] req.first;
	return resp;
}

unsigned char *_router::s_rsp_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	return nullptr;
}
/**
*	_router::make_find - Функция создания запросов к
*	нодам для поиска клиента.
*
*	@hash - Хэш искомого клиента.
*/
void _router::make_find(unsigned char *hash)
{
	using tgnstorage::routes;

	struct tgn_task task;
	struct tgn_ipport ip;
	unsigned char *msg_b;

	if (!hash || hash == nullptr
		|| routes.exists(hash) > 0)
		return;

	msg_b = msg_tmp<true>(S_REQUEST_FIND);
	msg_b[HASHSIZE + 1] = 0x00;

	memcpy(msg_b + HASHSIZE + 2, hash, HASHSIZE);
	memcpy(task.bytes, msg_b, HEADERSIZE);

	routes.add(hash, ip, true);

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
void _router::new_msg(struct tgn_ipport ipp,
	tgnmsg &msg, unsigned char *from)
{
	unsigned char *txt, *msg_b;
	struct tgn_task task;
	size_t hs = HASHSIZE;

	task.client_in = saddr_get(ipp.ip, ipp.port);
	task.target_only = true;
	task.length = FULLSIZE;

	msg_b = msg_tmp<false>(S_REQUEST_GARLIC);
	txt = msg.garlic_msg();

	if (from && from != nullptr) {
		memcpy(msg_b + hs * 2 + 1, from, hs);
	}
	memcpy(msg_b + HEADERSIZE, txt, TEXTSIZE);
	memcpy(task.bytes, msg_b, FULLSIZE);

	tgnstorage::tasks.add(task);

	delete[] msg_b;
	delete[] txt;
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
