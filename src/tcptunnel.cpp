
#include "../include/tcptunnel.hpp"
/*********************************************************/
void tcp_tunnel::start(bool is_send, size_t port) {
	auto &tunn = (is_send) ? send_init : recv_init;
	tunn.sock = new_socket(SOCK_STREAM, TIMEOUT);
	string ipaddr;
	bool conn;

	assert(port != 0 && tunn.sock != -1 && !tunn.start);

	role = (role == TCP_NONE) ? *init.role : role;
	tunn.start = true;
	work = true;

	conn = (role == TCP_BINDER2 && !is_send) || role % 2 != 0;

	if (role % 2 != 0) {
		HASHCPY(tunn.hash, init.node->hash);
		ipaddr = init.node->ipp.ip;
	}
	else {
		HASHCPY(tunn.hash, (role > TCP_BINDER1) ? init.user->hash
												: init.node->hash);
		ipaddr = (is_send) ? init.node->ipp.ip  : string();
	}

	set_sockaddr(tunn.own.sddr, port, ipaddr);
	bool chain = connect_chain(tunn, conn);

	auto func = (is_send) ? &tcp_tunnel::send_thread
						  : &tcp_tunnel::recv_thread;
	if (work && chain) {
		tunn.thr = thread(func, this, conn);
	}
}
/*********************************************************/
bool tcp_tunnel::connect_chain(tcp_tunnel::tcp_data &tunn,
							   bool connector) {
	socklen_t sz = sizeof(struct sockaddr_in);
	auto time = system_clock::now();
	bool cond = false;

	if (!connector) {
		int b = bind(tunn.sock, tunn.own.ptr, sz);
		int l = listen(tunn.sock, 5);

		work = (b + l != 0) ? false : true;
	}

	while (!cond) {
		if (system_clock::now() - time > 15s) {
			work = false;
			break;
		}

		if (!connector) {
			tunn.usock = accept(tunn.sock, tunn.srv.ptr,
								&sz);
			cond = tunn.usock < 0;
			continue;
		}

		cond = connect(tunn.sock, tunn.own.ptr, sz) < 0;
	}

	return cond;
}
/*********************************************************/
void tcp_tunnel::send_thread(bool connector) {
	auto time = system_clock::now();
	unsigned char flag, *encoded, *key;
	int fn_status, size, sock;

	key = (role % 2 != 0) ? init.node->hash
						  : init.user->hash;

	sock = (connector)	  ? send_init.sock
						  : send_init.usock;

// 0x00 - wait, 0x01 - ready
	while (time - send_init.time > 8s && work) {
		if (recv(sock, &flag, 1, 0) <= 0) {
			work = false;
			break;
		}

		if (flag == 0 || buff_size() == 0) {
			continue;
		}

		mute.lock();

		encoded = encryption::pack(key, buffer, length);
		size = length + crypto_box_SEALBYTES;
		time = system_clock::now();
		length = 0;

		fn_status = send(sock, encoded, size, 0);
		mute.unlock();

		delete[] encoded;

		work = (fn_status == -1) ? false : true;
	}
}
/*********************************************************/
void tcp_tunnel::recv_thread(bool connector) {
	auto time = system_clock::now();
	unsigned char flag, *decoded;

	int sock = (connector) ? recv_init.sock
						   : recv_init.usock;

	while (time - recv_init.time > 8s && work) {
		flag = (buff_size() == 0) ? 1 : 0;

		if (send(sock, &flag, 1, 0) < 0) {
			work = false;
			break;
		}

		if (flag == 0) {
			continue;
		}

		time = system_clock::now();
		mute.lock();

		length = recv(sock, buffer, TUNNPACK, 0);

		if (length <= 0) {
			work = false;
			mute.unlock();
			break;
		}

		length -= crypto_box_SEALBYTES;
		decoded = encryption::unpack(buffer, length);

		if (!decoded) {
			work = false;
			mute.unlock();
			break;
		}

		memcpy(buffer, decoded, length);
		delete[] decoded;

		mute.unlock();
	}
}
