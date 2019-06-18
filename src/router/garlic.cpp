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
*	_router::client_garlic - Функция обработки запроса
*	на отправку сообщения от клиента сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::client_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::routes;

	unsigned char *resp, *key = msg.byte_key();
	struct tgn_garlic req = msg.info_garlic();
	unsigned char *local_m;
	struct tgn_route route;
	struct tgn_task task;
	bool local = false;

	if (memcmp(key, req.to, HASHSIZE) == 0
		|| bytes_sum<HASHSIZE>(req.to) ==0) {
		delete[] key;
		return nullptr;
	}

	memcpy(req.from, key, HASHSIZE);

	if (routes.exists(req.to) != 1) {
		local = this->from_clients(req.to);
		this->from_neighbors(req.to);
	}

	if (!routes.find(route, req.to)) {
		resp = this->g_back(req, REQUEST_FIND, 0);
		this->make_find(req.to);

		delete[] key;
		return resp;
	}

	resp = this->g_back(req, GOOD_SERVER, 0);
	this->new_msg(route.ipport, msg, key, 0);

	if (local == false) {
		tgnstorage::garlic.add(req);
		delete[] key;
		return resp;
	}

	local_m = this->g_back(req, GOOD_TARGET, 0);
	memcpy(task.bytes, local_m, HEADERSIZE);

	task.client_in = saddr_get(route.ipport.ip, PORT);
	task.length = HEADERSIZE;
	task.target_only = true;

	tgnstorage::tasks.add(task);

	delete[] local_m;
	delete[] key;
	return resp;
}
/**
*	_router::node_garlic - Функция обработки запроса
*	на отправку сообщения от ноды сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::node_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::clients;

	struct tgn_garlic req = msg.info_garlic();
	struct tgn_ipport ipp = ipport_get(skddr);
	unsigned char *key = msg.byte_key();
	struct tgn_client client;

	if (!clients.find(client, req.to)) {
		delete[] key;
		return this->g_back(req, ERROR_TARGET, 1);
	}

	this->new_msg(client.ipport, msg, nullptr, 1);
	tgnstorage::routes.add(key, ipp, false);

	delete[] key;
	return this->g_back(req, GOOD_TARGET, 1);
}
/**
*	_router::status_garlic - Функция отправки статуса
*	сообщения обратно.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::status_garlic(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::clients;
	using tgnstorage::routes;
	using tgnstorage::tasks;

	unsigned char *resp, *key = msg.byte_key();
	struct tgn_garlic req = msg.info_garlic();
	struct sockaddr_in skddr_c;
	struct tgn_client client;
	struct tgn_task task;

	if (memcmp(key, req.to, HASHSIZE) == 0
		|| bytes_sum<HASHSIZE>(req.to) ==0
		|| !tgnstorage::garlic.exists(req)) {
		delete[] key;
		return nullptr;
	}

	if (!clients.find(client, req.from)) {
		tgnstorage::garlic.remove(req);

		delete[] key;
		return nullptr;
	}

	skddr_c = saddr_get(client.ipport.ip,
		client.ipport.port);
	tgnstorage::garlic.remove(req);

	task.client_in = skddr_c;
	task.length = HEADERSIZE;
	task.target_only = true;

	switch (req.status) {
	case GOOD_TARGET:
		resp = g_back(req, GOOD_TARGET, 0);
		memcpy(task.bytes, resp, HEADERSIZE);
		tasks.add(task);
		break;

	case ERROR_TARGET:
		resp = g_back(req, ERROR_TARGET, 0);
		memcpy(task.bytes, resp, HEADERSIZE);
		routes.remove_hash(req.to);
		tasks.add(task);
		break;

	default:
		routes.remove_hash(req.to);
		break;
	}

	return nullptr;
}
/**
*	_router::g_back - Функция формирования ответа на
*	запрос garlic.
*
*	@garlic - Структура информации о сообщении.
*	@s - Статус ответа сообщения.
*	@type - Тип получателя.
*/
unsigned char *_router::g_back(struct tgn_garlic garlic,
	enum tgn_status s, int type)
{
	unsigned char status = static_cast<unsigned char>(s);
	unsigned char *msg, *s_ptr;

	msg = msg_tmp<true>((type == 0) ? U_RESPONSE_GARLIC
		: S_RESPONSE_GARLIC);
	s_ptr = msg + HASHSIZE + 1;

	memcpy(s_ptr + HASHSIZE, garlic.to, HASHSIZE);
	memset(s_ptr + HASHSIZE + 1, status, 1);
	memcpy(s_ptr, garlic.from, HASHSIZE);

	return msg;
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
*	@from - Хэш отправителя.
*	@type - Тип получателя.
*/
void _router::new_msg(struct tgn_ipport ipp,
	tgnmsg &msg, unsigned char *from, int type)
{
	unsigned char *txt, *mb;
	struct tgn_task task;
	size_t hs = HASHSIZE;
	enum tgn_htype t;

	t = (type == 0) ? S_REQUEST_GARLIC : U_REQUEST_GARLIC;
	task.client_in = saddr_get(ipp.ip, ipp.port);
	task.target_only = true;
	task.length = FULLSIZE;

	mb = msg_tmp<false>(t);
	txt = msg.garlic_msg();

	if (from && from != nullptr) {
		memcpy(mb + hs * 2 + 1, from, hs);
	}
	task.bytes[HASHSIZE * 3 + 1] = EMPTY_STATUS;
	memcpy(mb + HEADERSIZE, txt, TEXTSIZE);
	memcpy(task.bytes, mb, FULLSIZE);

	tgnstorage::tasks.add(task);

	delete[] txt;
	delete[] mb;
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
bool _router::from_clients(unsigned char *hash)
{
	using tgnstorage::clients;
	using tgnstorage::routes;

	struct tgn_client client;

	if (!hash || !clients.find(client, hash))
		return false;

	routes.add(hash, client.ipport, false);
	return true;
}