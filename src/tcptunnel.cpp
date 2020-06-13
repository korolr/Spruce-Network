#include "../include/spruce.hpp" //// just add spruce?????????
#include "../include/storage.hpp"

void tcp_tunnel::init(struct init_tunnel initd,
					  struct tunnel *ptr) {
	if (!initd.hash || !ptr) {
		work = false;
		return;
	}

	HASHCPY(ehash, initd.hash);
	it = ptr;

	switch (initd.type) {
	case TCP_SND:
		this->sender(initd.role, initd.ipp);
		return;

	case TCP_RCV:
		this->receiver(initd.role, initd.ipp.port);
		return;

	default: work = false;
	}
}

tcp_tunnel::~tcp_tunnel(void) {
	if (thr.joinable()) { thr.join(); }
	CLOSE_SOCKET(sock);
	work = false;
}


void tcp_tunnel::sender(enum tcp_role r, struct ipport ipp) {
	socklen_t sz = sizeof(struct sockaddr_in);
	auto time = system_clock::now();

	sock = new_socket(SOCK_STREAM,  TIMEOUT);
	set_sockaddr(srv.sddr, ipp.port, ipp.ip);
	assert(ipp.port != 0 && sock != 0);

	it->time = system_clock::now();
	s_port = ipp.port;
	role = r;

	while (connect(sock, srv.ptr, sz) < 0) {
		if (system_clock::now() - time < 30s) {
			continue;
		}

		it->mute.lock();
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
		return;
	}

	thr = thread(&tcp_tunnel::thr_send, this);
}











void tcp_tunnel::receiver(enum tcp_role r, size_t port) {
	socklen_t sz = sizeof(struct sockaddr_in);
	auto time = system_clock::now();
	int csock = 0;

	sock = new_socket(SOCK_STREAM, TIMEOUT);
	assert(port != 0 && sock != 0);

	set_sockaddr(srv.sddr, port);

	it->mute.lock();
	it->status = TUNNEL_1;
	it->time = system_clock::now();
	it->mute.unlock();

	r_port = port;
	role = r;

	if (bind(sock, srv.ptr, sz) != 0) {
		cout << "[E]: Port is bussy.\n";
		work = false;
		return;
	}

	if (listen(sock, 5) != 0) {
		cout << "[E]: Can't start listening the socket.\n";
		work = false;
		return;
	}

	while ((csock = accept(sock, cln.ptr, &sz)) < 0) {
		if (system_clock::now() - time < 30s) {
			continue;
		}

		it->mute.lock();
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
		return;
	}

	it->mute.lock();

	switch (r) {
	case TCP_TARGET:
		work = false;
		it->mute.unlock();
		return;

	case TCP_BINDER1:
		it->status = TUNNEL_1;
		break;

	case TCP_BINDER2:
	case TCP_UBINDER:
		it->status = TUNNEL_2;
		break;

	default:
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
		return;
	}

	it->mute.unlock();

	thr = thread([&](int s) {
			unsigned char b[PACKLEN];
			size_t l;

			while (work) {
				l = recv(s, b, PACKLEN, 0);
				this->recv_processing(b, s, l);
			}
			CLOSE_SOCKET(s);
		}, 
		csock);
}


void tcp_tunnel::thr_send(void) {
	enum tcp_status st = CUR_STATUS, r_st = ERROR_T;
	bool bin2;
	size_t len;

	bin2 = role == TCP_BINDER2 || role == TCP_UBINDER;
	// Мы меняем текущий статус так как конечный
	// пользователь был подключен.
	if (bin2) {
		it->mute.lock();
		it->status = TUNNEL_3;
		it->mute.unlock();
	}

	while (work) {
		if (bin2) {
			this->send_processing(TUNNEL_3, bin2);
			continue;
		}

		if ((len = recv(sock, &r_st, 1, 0)) == 0) {
			it->mute.lock();
			it->status = ERROR_T;
			it->mute.unlock();
			work = false;
			return;
		}

		if (len == 1 && it->status >= TUNNEL_3
			&& r_st < TUNNEL_3) {
			it->mute.lock();
			it->status = ERROR_T;
			it->mute.unlock();
			work = false;
			return;
		}

		if (send(sock, &st, 1, 0) == -1
			|| !NORMAL_STATUS(r_st))  {
			it->mute.lock();
			it->status = ERROR_T;
			it->mute.unlock();
			work = false;
			return;
		}

		this->send_processing(r_st, bin2);
	}
}










void tcp_tunnel::recv_processing(unsigned char *buff,
								 int csock,
								 size_t len) {
	unsigned char st;

	if (system_clock::now() - it->time > 50s) {
		it->mute.lock();
		it->status = ERROR_S;
		it->mute.unlock();
		work = false;
		return;
	}

	if (len == 0) {
		it->mute.lock();
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
		return;
	}

	if (len == 1 && *buff == CUR_STATUS) {
		// Если в буффере есть записи - отдаем статус wait для
		// нижестоящего клиента т.к. нужно переслать имеющийся
		// буффер.
		it->mute.lock();
		st = (it->length == 0) ? it->status
							   : WAIT_SEND;
		if (send(sock, &st, 1, 0) == -1) {
			it->status = ERROR_T;
			work = false;
		}

		it->mute.unlock();
		return;
	}

	if ((len > 1 && it->status != TUNNEL_3)
		|| (len > 1 && it->length != 0)) {
		it->mute.lock();
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
		return;
	}

	it->mute.lock();
	memcpy(it->content, buff, len);
	it->time = system_clock::now();
	it->length = len;
	it->mute.unlock();
}








void tcp_tunnel::send_processing(enum tcp_status status,
								 bool is_bin2) {
	if (it->status != status && !is_bin2) {
		it->mute.lock();
		it->time = system_clock::now();
		it->status = status;
		it->mute.unlock();
	}

	if (system_clock::now() - it->time > 50s
		|| status == CUR_STATUS) {
		it->mute.lock();
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
		return;
	}

	switch (it->status) {
	case ERROR_S:
	case ERROR_T:
		work = false;
		break;

	case TUNNEL_1:
	case TUNNEL_2:
	case WAIT_SEND:
		break;

	case TUNNEL_3:
		if (it->length == 0) {
			return;
		}

		if (send(sock, it->content, it->length, 0) == -1) {
			it->mute.lock();
			it->status = ERROR_T;
			it->mute.unlock();
			work = false;
			return;
		}

		it->mute.lock();
		it->length = 0;
		it->mute.unlock();
		break;

	default:
		it->mute.lock();
		it->status = ERROR_T;
		it->mute.unlock();
		work = false;
	}
}
