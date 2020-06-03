#include "../include/spruce.hpp" //// just add spruce?????????
#include "../include/storage.hpp"

#define NORMAL_STATUS(c) ((c) >= TUNNEL_1 && (c) <= ERROR_T)

void tcp_tunnel::init(struct init_data initd,
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
	if (thr.joinable()) {
		thr.join();
	}

	if (sfd.is_open()) {
		sfd.close();
	}

	if (role == TCP_TARGET) {
		recvd.fd.close();
		storage::inbox.add(it->target, recvd.name);
	}

	if (role == TCP_SENDER) {
		storage::msgs.done(msg.id);
	}

	CLOSE_SOCKET(sock);
	work = false;
}


void tcp_tunnel::sender(enum tcp_role r, struct ipport ipp) {
	using storage::msgs;

	socklen_t sz = sizeof(struct sockaddr_in);
	auto time = system_clock::now();

	if (r == TCP_SENDER
		&& !msgs.find_hash(it->target, msg)) {
		work = false;
		return;
	}

	if (!msg.data.buff) {
		sfd.open(msg.data.name, ios::binary | ios::out);
		if (!sfd.good()) {
			work = false;
			return;
		}
	}

	sock = new_socket(SOCK_STREAM, TIMEOUT);
	assert(ipp.port != 0 && sock != 0);

	set_sockaddr(srv.sddr, ipp.port, ipp.ip);
	s_port = ipp.port;

	it->time = system_clock::now();
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
	char *name = nullptr;
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
		assert(name = tmpnam(nullptr));
		recvd.name = string(name);
		recvd.fd.open(recvd.name, ios::binary
								  | ios::in
								  | ios::app);
		assert(recvd.fd.is_open());
		it->status = TUNNEL_3;
		delete[] name;
		break;

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
	size_t len;

	while (work) {
		send(sock, &st, 1, 0);

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

		if (!NORMAL_STATUS(r_st)) {
			it->mute.lock();
			it->status = ERROR_T;
			it->mute.unlock();
			work = false;
			return;
		}

		this->send_processing(r_st);
	}
}










void tcp_tunnel::recv_processing(unsigned char *buff, int csock,
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
		send(sock, &st, 1, 0);
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

	if (role == TCP_TARGET) {
		recvd.fd.write(reinterpret_cast<char *>(buff), len);
	}
	else {
		memcpy(it->content, buff, len);
		it->length = len;
	}

	it->time = system_clock::now();
	it->mute.unlock();
}








void tcp_tunnel::send_processing(enum tcp_status status) {

	if (it->status != status) {
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

	this->get_content();

	switch (it->status) {
	case ERROR_D:
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

		send(sock, it->content, it->length, 0);
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


void tcp_tunnel::get_content(void) {
	size_t ssize = msg.data.ssize;
	size_t msize = msg.data.size;
	size_t nsize = msize - ssize;
	
	if (role != TCP_SENDER) {
		return;
	}

	assert(msg.data.buff);
	it->mute.lock();

	if (msg.data.size > 0) {
		if (nsize == 0) {
			it->status = ERROR_D;
			it->mute.unlock();
			return;
		}

		nsize = (nsize >= PACKLEN) ? PACKLEN
								   : nsize;
		memcpy(it->content, msg.data.buff
			   + ssize, nsize);
		msg.data.ssize += nsize;
		it->length = nsize;
		it->mute.unlock();
		return;
	}

	// from file --------------------------------------------
}
