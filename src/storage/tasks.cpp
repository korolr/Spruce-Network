
#include "../../include/storage.hpp"
/*********************************************************/
void tasks_handler::add(pack msg, struct sockaddr_in sddr,
						unsigned char *hash) {
	struct udp_task task;

	if (!msg.is_correct() || !hash) {
		return;
	}

	task = msg.to_task(hash, &sddr);
	task.time = system_clock::now();
	HASHCPY(task.hash, hash);
	task.attempts = 0;

	mute.lock();
	structs::tasks.push_back(task);
	mute.unlock();
}
/*********************************************************/
void tasks_handler::rm_hash(unsigned char *hash,
							size_t cookie) {
	vector<struct udp_task>::iterator it;
	int cmp;

	mute.lock();
	it = structs::tasks.begin();

	if (structs::tasks.size() == 0) {
		mute.unlock();
		return;
	}

	for (; it != structs::tasks.end(); it++) {
		cmp = memcmp((*it).hash, hash, HASHSIZE);

		if (cmp == 0 && (cookie == 0
			|| cookie == (*it).cookie)) {
			structs::tasks.erase(it);
			break;
		}
	}

	mute.unlock();
}
/*********************************************************/
void tasks_handler::rm_cookie(size_t cookie) {
	vector<struct udp_task>::iterator it;

	mute.lock();
	it = structs::tasks.begin();

	if (structs::tasks.size() == 0) {
		mute.unlock();
		return;
	}

	for (; it != structs::tasks.end(); it++) {
		if (cookie != (*it).cookie) {
			continue;
		}

		structs::tasks.erase(it);
		break;
	}

	mute.unlock();
}
/*********************************************************/
void tasks_handler::renew(void) {
	using structs::father;
	using structs::tasks;

	size_t size;

	mute.lock();
	size = tasks.size();

	for (size_t i = 0; i < size; i++) {
		if (tasks[i].type == USER_REQ_PING
			|| tasks[i].type == NODE_REQ_PING
			|| tasks[i].type == REQ_ROLE) {
			tasks.erase(tasks.begin() + i);
			continue;
		}

		if (structs::role == UDP_USER) {
			if (tasks[i].type != USER_REQ_TUNNEL
				&& tasks[i].type != USER_REQ_NODE) {
				tasks.erase(tasks.begin() + i);
				continue;
			}

			tasks[i].sddr = sddr_get(father.info.ipp);
		}
		

		tasks[i].time = system_clock::now() - 30s;
		tasks[i].attempts = 0;
	}

	mute.unlock();
}
/*********************************************************/
bool tasks_handler::exists(enum udp_type type) {
	bool status = false;

	mute.lock();

	for (auto &p : structs::tasks) {
		if (p.type == type && p.attempts < 3) {
			status = true;
			break;
		}
	}

	mute.unlock();
	return status;
}
/*********************************************************/
bool tasks_handler::first(struct udp_task &one) {
	using structs::tasks;

	bool status = false;
	size_t id = 0, size;

	mute.lock();

	if ((size = tasks.size()) == 0) {
		mute.unlock();
		return status;
	}

	one = *tasks.begin();

	for (size_t i = 0; i < size; i++) {
		if (tasks[i].type == NODE_RES_PING
			&& tasks[i].attempts < 3) {
			one = tasks[i];
			status = true;
			id = i;
			break;
		}

		if (tasks[i].attempts >= 3
			|| one.time < tasks[i].time) {
			// Если много сообщений для отца - смена
			continue;
		}

		one = tasks[i];
		status = true;
		id = i;
	}

	tasks[id].time = system_clock::now();
	tasks[id].attempts++;

	if (one.type % 2 == 0) {
		tasks.erase(tasks.begin() + id);
	}

	mute.unlock();
	return status && one.attempts <= 3;
}
/*********************************************************/
void tasks_handler::check(void) {
	using structs::tasks;

	string ip = structs::father.info.ipp.ip;
	auto time = system_clock::now();
	size_t num = 0, size;
	struct ipport ipp;

	mute.lock();

	if ((size = structs::tasks.size()) == 0) {
		mute.unlock();
		return;
	}

	for (size_t i = 0; i < size; i++) {
		if (time - tasks[i].time > 600s
			&& tasks[i].attempts >= 3) {
			tasks.erase(tasks.begin() + i);
		}
	}

	for (auto &p : tasks) {
		if (ipport_get(p.sddr).ip == ip
			&& p.attempts >= 3) {
			num++;
		}
	}

	mute.unlock();

	if (num > 15) {
		storage::father.no_father();
	}
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void tasks_handler::print(void) {
	struct ipport ipp;

	cout << "Attempts, Type, Cookie, Ip" << endl
		 << "--------------------------" << endl;

	mute.lock();

	for (auto &p : structs::tasks) {
		ipp = ipport_get(p.sddr);

		cout << p.attempts << ", " << p.type << ", "
			 << p.cookie << ", " << ipp.ip << endl;
	}

	mute.unlock();
	cout << endl << endl;
}

#endif