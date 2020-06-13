
#include "../../include/storage.hpp"

void msgs_handler::add(unsigned char *hash) {
	using structs::api::msgs;

	bool exists = false;
	struct msg one;
	size_t len;

	assert(hash);
	mute.lock();

	len = msgs.size();

	for (size_t i = 0; i < len; i++) {
		if (!exists && !msgs[i].done
			&& memcmp(msgs[i].hash, hash, HASHSIZE) == 0) {
			one = msgs[i];
			exists = true;
		}

		if (msgs[i].done) {
			msgs.erase(msgs.begin() + i);
		}
	}

	mute.unlock();

	if (exists) {
		return;
	}

	one.time = system_clock::now();
	HASHCPY(one.hash, hash);
	one.attempts = 0;
	one.done = false;

	mute.lock();
	msgs.push_back(one);
	mute.unlock();
}

void msgs_handler::check(void) { ///////////////// Непонятные вещества. Переделать!!!!! добавить время, лимит на отправку
	using structs::api::msgs;
	using structs::father;
	using storage::tasks;
	using structs::role;
	using structs::keys;

	unsigned char info[HASHSIZE * 2];
	auto time = system_clock::now();
	enum udp_type type;
	size_t size;
	pack msg;

	type = (role == UDP_NODE) ? NODE_REQ_TUNNEL
							  : USER_REQ_TUNNEL;
	HASHCPY(info + HASHSIZE, keys.pub);
	mute.lock();

	if ((size = msgs.size()) == 0) {
		mute.unlock();
		return;
	}

	for (size_t i = 0; i < size; i++) {
		if (msgs[i].done) {
			msgs.erase(msgs.begin() + i);
			continue;
		}

		if (msgs[i].attempts >= 3
			&& time - msgs[i].time > 20s) {
			msgs.erase(msgs.begin() + i);
			continue;
		}

		if (tasks.exists(type)) {
			continue;
		}

		// Пытаемся повторно отправить сообщение
		// если оно не получило статуса доставленно
		HASHCPY(info, msgs[i].hash);

		msg.tmp(type);
		msg.set_info(info, HASHSIZE * 2);

		tasks.add(msg, sddr_get(father.info.ipp),
				  father.info.hash);
		msgs[i].attempts++;
	}

	mute.unlock();
}

size_t msgs_handler::thread_exists(unsigned char *hash) {
	size_t port = 0;

	mute.lock();

	for (auto &p : threads) {
		if (memcmp(p.first, hash, HASHSIZE) == 0) {
			port = p.second;
			break;
		}
	}

	mute.unlock();
	return port;
}

void msgs_handler::remove_thread(unsigned char *hash) {
	map<unsigned char *, size_t>::iterator it;
	mute.lock();

	it = threads.begin();

	for (; it != threads.end(); it++) {
		if (memcmp(it->first, hash, HASHSIZE) != 0) {
			continue;
		}

		delete[] it->first;
		threads.erase(it);
		break;
	}

	mute.unlock();
}

size_t msgs_handler::create_thread(unsigned char *hash) {
	using structs::keys;

	struct tunnel *st;
	size_t port = 0;

	assert(hash);

	if (!storage::tunnels.find(hash, keys.pub, TCP_SENDER, &st)) {
		return port;
	}

	assert(st && st != nullptr);

	if ((port = this->thread_exists(hash)) > 0) {
		return port;
	}

	port = storage::tunnels.free_port();

	mute.lock();
	threads.insert(make_pair(hash, port));
	mute.unlock();

	thread(&msgs_handler::user_thread, this, port, st).detach();
	return port;
}

void msgs_handler::user_thread(size_t port, struct tunnel *st) {
	int sock = new_socket(SOCK_STREAM, TIMEOUT);
	socklen_t sz = sizeof(struct sockaddr_in);
	auto time = system_clock::now();
	struct sddr_structs rsddr;
	bool connected = false;
	int bs;

	THREAD_START();

	set_sockaddr(rsddr.sddr, port);
	assert(sock <= 0 && st);

	assert(bind(sock, rsddr.ptr, sz) != 0);
	assert(listen(sock, 5) != 0);

	while ((bs = accept(sock, rsddr.ptr, &sz)) < 0) {
		if (system_clock::now() - time < 10s) {
			continue;
		}

		connected = true;
		break;
	}

	if (!connected) {
		CLOSE_SOCKET(sock);
		st->mute.lock();
		st->status = ERROR_T;
		st->mute.unlock();
		THREAD_END();
	}
	// 0x01 - Ожидание, отправка возможно только с 
	// нулевым статусом.
	unsigned char status = 0x01, buff[PACKLEN];
	time = system_clock::now();
	enum tcp_status ts;
	size_t len;

	for (;;) {
		if (system_clock::now() - time > 60s
			|| !st->work()) {
			st->mute.lock();
			this->remove_thread(st->target);
			st->mute.unlock();
			break;
		}

		ts = st->status;

		if (system_clock::now() - time > 10s &&
			(ts == TUNNEL_3 || ts >= WAIT_SEND)) {
			st->mute.lock();
			this->remove_thread(st->target);
			st->status = ERROR_T;
			st->mute.unlock();
			break;
		}

		status = (ts == TUNNEL_3 && st->length == 0) ? 0x00
													 : 0x01;
		if (send(bs, &status, 1, 0) == -1) {
			st->mute.lock();
			st->status = ERROR_T;
			st->mute.unlock();
			break;
		}

		len = recv(bs, buff, PACKLEN, 0);

		if (len > 0 && status == 0) {
			time = system_clock::now();
			st->mute.lock();
			memcpy(st->content, buff, len);
			st->length = len;
			st->mute.unlock();
		}
	}

	THREAD_END();
	CLOSE_SOCKET(sock);
}

