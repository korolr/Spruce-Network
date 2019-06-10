/**
*	namespace.cpp - Модуль отвечающий за работу
*	функций пространства имен tinrouter.
*
*	@mrrva - 2019
*/
#include "../include/router.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	tinrouter::client - Функция обработки сообщений
*	от клиентов сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
struct tin_task tinrouter::client(tinmsg msg,
		struct sockaddr_in skddr)
{
	enum tin_htype type = msg.header_type();
	struct tin_task task;

	if (type == INDEFINITE_MESSAGE || type >= 0x10
		|| type == U_REQUEST_DOS) {
		task.bytes[0] = 0x00;
		return task;
	}

	switch(type) {
		case U_REQUEST_NODES:
			task = router.su_nodes(msg, skddr, true);
			break;

		case U_REQUEST_GARLIC:
			task = router.u_garlic(msg, skddr);
			break;

		default:
			task.bytes[0] = 0x00;
			break;
	}

	return task;
}
/**
*	tinrouter::node - Функция обработки сообщений
*	от нод сети.
*
*	@msg - Сообщение от ноды сети.
*	@skddr - Структура sockaddr_in.
*/
struct tin_task tinrouter::node(tinmsg msg,
	struct sockaddr_in skddr)
{
	enum tin_htype type = msg.header_type();
	struct tin_task task;

	if (type == INDEFINITE_MESSAGE || type <= 0x10
		|| type == S_REQUEST_DOS) {
		task.bytes[0] = 0x00;
		return task;
	}

	tinstorage::nodes.ping(skddr);

	switch (type) {
		case S_REQUEST_NODES:
			task = router.su_nodes(msg, skddr, false);
			break;

		case S_REQUEST_FIND:
			task = router.s_find(msg, skddr);
			break;

		case S_REQUEST_GARLIC:
			task = router.s_garlic(msg, skddr);
			break;

		case S_REQUEST_CLIENTS:
			task = router.s_neighbors(msg, skddr);
			break;

		default:
			task.bytes[0] = 0x00;
			break;
	}

	return task;
}