
#include "../../include/storage.hpp"

bool ports_handler::reg(size_t port, unsigned char *hash) {
	vector<struct tcp_port>::iterator it;
	struct tcp_port one;
	bool s;

	assert(hash);

	if (port < 49160) {
		return false;
	}

	mute.lock();
	s = vector_search(it, structs::ports, hash);

	if (s && it->port == port) {
		it->confirm = true;
		mute.unlock();
		return true;
	}

	if (s || !is_free(port))   {
		mute.unlock();
		return false;
	}

	one.id = byte_sum(hash, HASHSIZE);
	one.time = system_clock::now();
	HASHCPY(one.hash, hash);
	one.confirm = true;
	one.port = port;

	vector_push(structs::ports, one);

	mute.unlock();
	return true;
}

size_t ports_handler::try_reg(unsigned char *hash) {
	vector<struct tcp_port>::iterator it;
	struct tcp_port one;
	bool s;

	assert(hash);

	mute.lock();
	s = vector_search(it, structs::ports, hash);

	if (s) {
		one.port = it->port = this->free();
		mute.unlock();
		return one.port;
	}

	one.id = byte_sum(hash, HASHSIZE);
	one.time = system_clock::now();
	HASHCPY(one.hash, hash);
	one.port = this->free();

	vector_push(structs::ports, one);

	mute.unlock();
	return one.port;
}

bool ports_handler::find(unsigned char *hash,
						 struct tcp_port &one) {
	vector<struct tcp_port>::iterator it;
	bool s;

	mute.lock();
	s = vector_search(it, structs::ports, hash);
	mute.unlock();

	if (s) {
		one = *it;
	}

	return s;
}

bool ports_handler::exists(unsigned char *hash) {
	vector<struct tcp_port>::iterator it;
	bool s;

	assert(hash);

	mute.lock();
	s = vector_search(it, structs::ports, hash);
	mute.unlock();

	return s;
}

void ports_handler::rm_port(size_t port) {
	vector<struct tcp_port>::iterator it;

	assert(port >= 49160);
	mute.lock();

	it = structs::ports.begin();

	for (; it != structs::ports.end(); it++) {
		if (it->port == port) {
			structs::ports.erase(it);
			break;
		}
	}

	mute.unlock();
}

size_t ports_handler::free(void) {
	using structs::ports;

	vector<size_t>::iterator it;
	vector<size_t> list;
	size_t port = 49160;

	for (auto &p : structs::ports) {
		list.push_back(p.port);
	}

	for (;; port++) {
		it = std::find(list.begin(), list.end(), port);
		assert(port != 0);

		if (it == list.end() && sys_free(port)) {
			break;
		}
	}

	return port;
}

bool ports_handler::sys_free(size_t port) {
	int tsock = socket(AF_INET, SOCK_STREAM, 0);
	socklen_t sz = sizeof(struct sockaddr_in);
	bool status;

	st.sddr.sin_port = htons(port);

	status = bind(tsock, st.ptr, sz) == 0;
	CLOSE_SOCKET(tsock);

	return status;
}

bool ports_handler::is_free(size_t port) {
	vector<size_t>::iterator it;
	vector<size_t> list;

	assert(port != 0);

	for (auto &p : structs::ports) {
		list.push_back(p.port);
	}

	it = std::find(list.begin(), list.end(), port);
	return it == list.end() && sys_free(port);
}

void ports_handler::check(void) {
	using structs::ports;

	auto rmc = [](struct tcp_port &one) {
		return system_clock::now() - one.time >= 20s;
	};

	mute.lock();
	ports.erase(remove_if(ports.begin(), ports.end(),
						  rmc), ports.end());
	mute.unlock();
}