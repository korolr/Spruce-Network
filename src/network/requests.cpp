/**
*	requests.cpp - Модуль отвечающий за новые
*	запросы пользователей сети TIN.
*
*	@mrrva - 2019
*/
#include "../include/network.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_requests::operator << - Оператор объекта для
*	запуска нового потока обработки сообщения.
*
*	@dt - Структура входящих параметров для потока.
*/
void _requests::operator <<(struct tin_task dt)
{
	tinmsg msg(dt.bytes);

	if (msg.is_node()) {
		thread(&_requests::thr_node, this, msg,
			dt.client_in).detach();
		return;
	}

	thread(&_requests::thr_client, this, msg,
		dt.client_in).detach();
}
/**
*	_requests::thr_client - Функция потока обработки
*	сообщений от клиентов сети.
*
*	@msg - Сообщение пользователя.
*	@skddr - Структура подключения пользователя.
*/
void _requests::thr_client(tinmsg msg,
	struct sockaddr_in skddr)
{
	unsigned char *resp, *hash = msg.byte_key();
	enum tin_htype type = msg.header_type();
	struct tin_task response;
	struct tin_ipport ipport;

	if (type == INDEFINITE_MESSAGE)
		return;

	ipport = ipport_get(skddr);
	tinstorage::clients.update(hash, ipport);

	if ((resp = msg_usr<true>(type)) == nullptr)
		resp = msg_tmp<true>(U_RESPONSE_DOS);

	this->socket_response(resp, skddr);
	response = tinrouter::client(msg, skddr);

	if (response.bytes[0] != 0x00)
		tinstorage::tasks.add(response);

	if (hash != nullptr)
		delete[] hash;
	delete[] resp;
}
/**
*	_requests::thr_node - Функция потока обработки
*	сообщений от нод сети.
*
*	@msg - Сообщение пользователя.
*	@skddr - Структура подключения пользователя.
*/
void _requests::thr_node(tinmsg msg,
	struct sockaddr_in skddr)
{
	struct tin_ipport ipport = ipport_get(skddr);
	enum tin_htype type = msg.header_type();
	struct tin_task response;
	unsigned char *resp, *hash;
	struct tin_node node;

	if (type == INDEFINITE_MESSAGE
		|| ipport.port != PORT)
		return;

	if ((resp = msg_usr<true>(type)) == nullptr)
		resp = msg_tmp<true>(S_RESPONSE_DOS);

	this->socket_response(resp, skddr);
	response = tinrouter::node(msg, skddr);

	if (response.bytes[0] != 0x00)
		tinstorage::tasks.add(response);

	if ((hash = msg.byte_key()) == nullptr) {
		delete[] resp;
		return;
	}

	memcpy(node.hash, hash, HASHSIZE);
	node.ip = ipport.ip;

	tinstorage::nodes.add(node);
	delete[] resp;
	delete[] hash;
}
/**
*	_requests::socket_response - Функция отправки ответа
*	пользователю сети.
*
*	@buffer - Байтовый массив сообщения.
*	@sin - Структура sockaddr_in.
*/
void _requests::socket_response(unsigned char *buffer,
	struct sockaddr_in &sin)
{
	using tinnetwork::sok;

	size_t sz = sizeof(struct sockaddr_in);
	struct sockaddr *saddr;

	if (buffer == nullptr)
		return;

	saddr = reinterpret_cast<struct sockaddr *>(&sin);
	sendto(sok, buffer, HEADERSIZE, 0x100, saddr, sz);
}
