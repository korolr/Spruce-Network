/**
*	socket.cpp - Модуль отвечающий за сокеты
*	децентрализованной сети TGN.
*
*	@mrrva - 2019
*/
#include "../include/network.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_socket::_socket - Конструктор класса _socket. Заполняет
*	переменные и указатели.
*/
_socket::_socket(void)
{
	using tgnnetwork::sok;

	this->s_data.srv.sin_addr.s_addr = INADDR_ANY;
	this->s_data.srv.sin_port = htons(PORT);
	this->s_data.srv.sin_family = AF_INET;

	if ((sok = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
		cout << "[E] _network::_network: start "
			<< "socket.\n";
		exit(1);
	}

	this->s_data.p_srv = reinterpret_cast<struct sockaddr *>(
		&this->s_data.srv);
	this->set_opts(sok);
}
/**
*	_socket::~_socket - Деструктор класса _socket. Уничтожает
*	переменные и указатели.
*/
_socket::~_socket(void)
{
	this->stop = true;

	if (tgnnetwork::sok != 0) {
		shutdown(tgnnetwork::sok, SHUT_RDWR);
		close(tgnnetwork::sok);
	}
}
/**
*	_socket::set_opts - Функция задающая параметры сокету
*	сети.
*
*	@stream - Ссылка на сокет.
*/
void _socket::set_opts(int &stream)
{
	int sol = SOL_SOCKET, opt;
	char *t_opt;

	if (stream == 0) {
		cout << "[E] _socket::set_opts.\n";
		return;
	}

	t_opt = reinterpret_cast<char *>(&this->timeout);
	this->timeout.tv_sec = TIMEOUT;
	opt = sizeof(this->timeout);
	this->timeout.tv_usec = 0;

	setsockopt(stream, sol, SO_RCVTIMEO, t_opt, opt);
	setsockopt(stream, sol, SO_SNDTIMEO, t_opt, opt);
}
/**
*	_socket::start - Функция запускающая потоки приема
*	и передачи данных.
*/
bool _socket::start(void)
{
	using tgnnetwork::send;
	using tgnnetwork::recv;

	send = thread(&_socket::thread_send, this);
	recv = thread(&_socket::thread_recv, this);

#ifdef TGN_DEBUG
	cout << "[S] Threads started.\n";
#endif
	if (send.joinable() && recv.joinable())
		return true;
	return false;
}
/**
*	_socket::thread_recv - Функция потока приема данных
*	сети.
*/
void _socket::thread_recv(void)
{
	using tgnnetwork::sok;

	unsigned char buff[FULLSIZE];
	struct tgn_task request;
	struct sockaddr_in user;
	struct sockaddr *cl;
	socklen_t sz;

	cl = reinterpret_cast<struct sockaddr *>(&user);
	sz = sizeof(struct sockaddr_in);
	bind(sok, this->s_data.p_srv, sz);
	memset(buff, 0x00, FULLSIZE);

	while (!this->stop) {
		recvfrom(sok, buff, FULLSIZE, 0x100, cl, &sz);

		if (bytes_sum<FULLSIZE>(buff) == 0x00)
			continue;
		memcpy(request.bytes, buff, FULLSIZE);
		memset(buff, 0x00, FULLSIZE);
		request.client_in = user;

		tgnnetwork::requests << request;
	}
}
/**
*	_socket::send_task - Функция циклической отправки
*	пакета задания.
*
*	@t - Структура текущего задания.
*/
void _socket::send_task(struct tgn_task &t)
{
	using tgnnetwork::sok;

	struct sockaddr_in u = t.client_in;
	struct sockaddr *cl;
	string ipaddr;
	size_t i = 0;
	socklen_t sz;

	cl = reinterpret_cast<struct sockaddr *>(&u);
	sz = sizeof(struct sockaddr_in);
	u.sin_family = AF_INET;

	do {
		sendto(sok, t.bytes, t.length, 0x100, cl, sz);

		if (i >= tgnstruct::nodes.size()) {
			break;
		}

		ipaddr = tgnstruct::nodes[i++].ip;
		u.sin_addr.s_addr = inet_addr(ipaddr.c_str());
	}
	while (!t.target_only);
}
/**
*	_socket::thread_send - Функция потока для создания
*	потоков отправки сообщений.
*/
void _socket::thread_send(void)
{
	using tgnstruct::tasks;

	while (!this->stop) {
		if (tasks.empty())
			continue;

		this->send_task(*tasks.begin());
		tgnstorage::tasks.remove_first();
	}
}