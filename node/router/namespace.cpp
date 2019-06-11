/**
*	namespace.cpp - Модуль отвечающий за работу
*	функций пространства имен tgnrouter.
*
*	@mrrva - 2019
*/
#include "../include/router.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	tgnrouter::client - Функция обработки сообщений
*	от клиентов сети.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*/
struct tgn_task tgnrouter::client(tgnmsg msg,
		struct sockaddr_in skddr)
{
	enum tgn_htype type = msg.header_type();
	struct tgn_task task;

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
*	tgnrouter::node - Функция обработки сообщений
*	от нод сети.
*
*	@msg - Сообщение от ноды сети.
*	@skddr - Структура sockaddr_in.
*/
struct tgn_task tgnrouter::node(tgnmsg msg,
	struct sockaddr_in skddr)
{
	enum tgn_htype type = msg.header_type();
	struct tgn_task task;

	if (type == INDEFINITE_MESSAGE || type <= 0x10
		|| type == S_REQUEST_DOS) {
		task.bytes[0] = 0x00;
		return task;
	}

	tgnstorage::nodes.ping(skddr);

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