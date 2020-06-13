
#include "../../include/storage.hpp"



size_t messages_handler::add_bytes(unsigned char *hash,
								   unsigned char *bytes,
								   size_t size) {
	struct tcp_message one;

	one.data.buff = new unsigned char[size];

	assert(bytes && hash && size > 0);
	assert(size <= PACKLEN);
	assert(one.data.buff);

	memcpy(one.data.buff, bytes, size);
	one.time = system_clock::now();
	one.id = this->free_id();
	HASHCPY(one.hash, hash);
	one.data.size = size;
	one.attempts = 0;
	one.done = false;

	mute.lock();
	structs::msgs.push_back(one);
	mute.unlock();

	return one.id;
}

size_t messages_handler::add_file(unsigned char *hash,
								  string name) {
	struct tcp_message one;

	assert(hash && name.length() > 4);

	one.time = system_clock::now();
	one.id = this->free_id();
	one.data.buff = nullptr;
	HASHCPY(one.hash, hash);
	one.data.name = name;
	one.data.ssize = 0;
	one.data.size = 0;
	one.attempts = 0;
	one.done = false;

	mute.lock();
	structs::msgs.push_back(one);
	mute.unlock();

	return one.id;
}

size_t messages_handler::free_id(void) {
	vector<size_t>::iterator it;
	vector<size_t> ids;
	size_t free = 1;

	mute.lock();

	for (auto &p : structs::msgs) {
		ids.push_back(p.id);
	}

	for (;; free++) {
		it = find(ids.begin(), ids.end(), free);
		assert(free != 0);

		if (it == ids.end()) {
			break;
		}
	}

	mute.unlock();
	return free;
}

void messages_handler::done(size_t id) {
	mute.lock();

	for (auto &p : structs::msgs) {
		if (p.id == id) {
			p.time = system_clock::now();
			p.done = true;
			break;
		}
	}

	mute.unlock();
}

void messages_handler::rm_id(size_t id) {
	vector<struct tcp_message>::iterator it;

	mute.lock();
	it = structs::msgs.begin();

	for (; it != structs::msgs.end(); it++) {
		if ((*it).id != id) {
			continue;
		}

		if ((*it).data.buff) {
			delete[] (*it).data.buff;
		}

		structs::msgs.erase(it);
		break;
	}

	mute.unlock();
}

bool messages_handler::find_hash(unsigned char *hash,
								 struct tcp_message &one) {
	bool status = false;

	assert(hash);

	mute.lock();
	
	for (auto &p : structs::msgs) {
		if (memcmp(hash, p.hash, HASHSIZE) != 0) {
			continue;
		}

		status = true;
		one = p;
		break;
	}

	mute.unlock();
	return status;
}

size_t messages_handler::info(size_t id) {
	struct tcp_message one;
	bool status = false;

	assert(id != 0);

	mute.lock();

	for (auto &p : structs::msgs) {
		if (p.id == id) {
			status = true;
			one = p;
			break;
		}
	}

	mute.unlock();

	if (!status || one.done) {
		return 0;
	}

	return one.attempts;
}

void messages_handler::check(void) {
	using structs::father;
	using storage::tasks;
	using structs::msgs;
	using structs::role;
	using structs::keys;

	unsigned char info[HASHSIZE * 2];
	auto time = system_clock::now();
	enum udp_type type;
	size_t size;
	pack msg;

	type = (role == UDP_NODE) ? NODE_REQ_TUNNEL
							  : USER_REQ_TUNNEL;
	memcpy(info + HASHSIZE, keys.pub, HASHSIZE);
	mute.lock();

	if ((size = msgs.size()) == 0) {
		mute.unlock();
		return;
	}

	for (size_t i = 0; i < size; i++) {
		if (msgs[i].done
			&& time - msgs[i].time > 20s) {
			if (msgs[i].data.buff) {
				delete[] msgs[i].data.buff;
			}
			msgs.erase(msgs.begin() + i);
			continue;
		}

		if (msgs[i].attempts >= 3
			&& time - msgs[i].time > 20s) {
			if (msgs[i].data.buff) {
				delete[] msgs[i].data.buff;
			}
			msgs.erase(msgs.begin() + i);
			continue;
		}

		if (msgs[i].attempts > 3
			|| msgs[i].done
			|| time - msgs[i].time < 10s
			|| tasks.exists(type)) {
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