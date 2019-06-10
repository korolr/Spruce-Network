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
*	_router::s_find - Функция обработки полученных
*	запросов на нахождение клиента в сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
struct tin_task _router::s_find(tinmsg &msg,
	struct sockaddr_in &skddr)
{
	using tinstorage::routes;

	struct find_request f_req = msg.info_find();
	unsigned char *msg_b, *info;
	struct tin_ipport ipp;
	struct tin_task task;
	string target;

	if (bytes_sum<HASHSIZE>(f_req.hash) == 0x00) {
		task.bytes[0] = 0x00;
		return task;
	}

	msg_b = msg_tmp<true>(S_REQUEST_FIND);
	ipp = ipport_get(skddr);
	target = f_req.target;
	info = msg.info_msg();

	if (f_req.found == false) {
		memcpy(msg_b + 1 + HASHSIZE, info, INFOSIZE);
		task = this->cycle_find(msg_b, skddr, f_req);

		delete[] msg_b;
		delete[] info;
		return task;
	}

	if (f_req.target.length() < 6) {
		target = ipp.ip;
	}

	if (routes.exists(f_req.hash) == 1) {
		routes.update(f_req.hash, {target, PORT});
	}

	memcpy(msg_b + 1 + HASHSIZE, info, INFOSIZE);
	task = this->send_find(msg_b, f_req, target);

	delete[] msg_b;
	delete[] info;
	return task;
}
/**
*	_router::send_find - Функция отправки запроса обратно
*	если клиент был найден.
*
*	@msg - Сообщение от клиента сети.
*	@req - Обработанная структура поиска.
*	@target - Откуда пришел запрос.
*/
struct tin_task _router::send_find(unsigned char *msg,
	struct find_request req, string target)
{
	struct tin_task task;
	unsigned char *tmp;

	if (!msg || target.length() < 6
		|| req.from.length() < 6) {
		task.bytes[0] = 0x00;
		return task;
	}

	tmp = iptobytes(target);

	memcpy(msg + 2 + HASHSIZE * 2, tmp, 4);
	memcpy(task.bytes, msg, HEADERSIZE);

	task.client_in = saddr_get(req.from, PORT);
	task.length = HEADERSIZE;
	task.target_only = true;

	delete[] tmp;
	return task;
}
/**
*	_router::cycle_find - Функция циклической отправки
*	запроса на нахождение ноды.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*	@req - Обработанная структура поиска.
*/
struct tin_task _router::cycle_find(unsigned char *msg,
	struct sockaddr_in skddr, struct find_request req)
{
	struct tin_ipport ipport = ipport_get(skddr);
	struct tin_task task;
	unsigned char *tmp;

	if (!msg || msg == nullptr) {
		task.bytes[0] = 0x00;
		return task;
	}

	if (req.from.length() < 6) {
		tmp = iptobytes(ipport.ip);

		memcpy(msg + 6 + HASHSIZE * 2, tmp, 4);
		delete[] tmp;
	}

	memcpy(task.bytes, msg, HEADERSIZE);

	if (tinstorage::clients.exists(req.hash)) {
		task.bytes[HASHSIZE + 1] = 0x01;
		task.length = HEADERSIZE;
		task.target_only = true;
		task.client_in = skddr;

		return task;
	}

	for (auto &p : tinstruct::nodes) {
		task.client_in = saddr_get(p.ip, PORT);
		task.length = HEADERSIZE;
		task.target_only = true;

		tinstorage::tasks.add(task);
	}

	task.bytes[0] = 0x00;
	return task;
}
