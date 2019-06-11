
#include "include/network.hpp"

using namespace std;

_network::_network(void)
{
	this->sock = socket(AF_INET, SOCK_DGRAM, 0);
	this->s_data.srv.sin_port = htons(PORT);
	this->s_data.srv.sin_family = AF_INET;
	this->stop = false;

	if (this->sock == 0) {
		cout << "[E] _network::_network: start "
			<< "socket.\n";
		exit(1);
	}

	s_data.p_srv = reinterpret_cast<struct sockaddr *>(
		&this->s_data.srv);
	this->set_opts(this->sock);
}
/**
*	_socket::~_socket - Деструктор класса _socket. Уничтожает
*	переменные и указатели.
*/
_socket::~_socket(void)
{
	this->stop = true;

	if (this->sock != 0) {
		shutdown(this->sock, SHUT_RDWR);
		close(this->sock);
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