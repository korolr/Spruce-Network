/**
*	find.cpp - Часть модуля маршрутизации в котором
*	хранятся функции обработки запросов на поиск клиентов.
*
*	@mrrva - 2019
*/
#include "../include/router.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_router::req_find - Функция обработки полученных
*	запросов на нахождение клиента в сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::req_find(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::clients;

	struct tgn_find_req req = msg.info_find();
	const short hs = HASHSIZE;
	struct sockaddr_in s_addr;
	struct tgn_ipport ipport;
	unsigned char *msb, *tmp;
	struct tgn_task task;

	if (bytes_sum<hs>(req.hash) == 0x00)
		return nullptr;

	if (clients.exists(req.hash)) {
		msb = msg_tmp<true>(S_RESPONSE_FIND);
		memcpy(msb + hs + 1, req.hash, hs);

		if (req.from.length() < 6
			|| req.from == "0.0.0.0")
			return msb;

		memcpy(task.bytes, msb, HEADERSIZE);
		memset(msb + hs+ 1, 0x00, INFOSIZE);
		s_addr = saddr_get(req.from, PORT);

		task.length = HEADERSIZE;
		task.target_only = true;
		task.client_in = s_addr;

		tgnstorage::tasks.add(task);
		return msb;
	}

	msb = msg_tmp<true>(S_REQUEST_FIND);
	ipport = ipport_get(skddr);

	if (req.from.length() < 6
		|| req.from == "0.0.0.0") {
		tmp = iptobytes(ipport.ip);
		memcpy(msb + hs * 2 + 5, tmp, 4);
		delete[] tmp;
	}

	memcpy(task.bytes, msb, HEADERSIZE);
	memset(msb + hs+ 1, 0x00, INFOSIZE);
	task.length = HEADERSIZE;
	task.target_only = true;

	for (auto &p : tgnstruct::nodes) {
		task.client_in = saddr_get(p.ip, PORT);
		tgnstorage::tasks.add(task);
	}

	return msb;
}
/**
*	_router::rsp_find - Функция обработки полученных
*	ответов на запрос нахождение клиента в сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
unsigned char *_router::rsp_find(tgnmsg &msg,
	struct sockaddr_in &skddr)
{
	using tgnstorage::routes;

	struct tgn_find_req req = msg.info_find();
	struct tgn_ipport ipp = ipport_get(skddr);

	if (bytes_sum<HASHSIZE>(req.hash) == 0x00)
		return nullptr;

	if (routes.exists(req.hash) == 1) {
		routes.update(req.hash, {ipp.ip, PORT});
	}

	return nullptr;
}
