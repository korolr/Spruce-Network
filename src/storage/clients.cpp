
#include "../../include/storage.hpp"
/*********************************************************/
void clients_handler::reg(unsigned char *hash,
						  struct ipport ipp,
						  enum udp_role role,
						  bool is_ping) {
	using structs::keys;

	bool is_node = false;
	struct client one;
	
	if (!hash || role == UDP_NONE || IS_ME(hash)) {
		return;
	}

	if (ipp.port == UDP_PORT && role == UDP_NODE) {
		is_node = true;
	}

	mute.lock();

	for (auto &p : structs::clients) {
		if (ipp.ip != p.ipp.ip) {
			continue;
		}

		p.ping = system_clock::now();
		HASHCPY(p.hash, hash);
		p.is_node = is_node;
		mute.unlock();
		return;
	}

	if (!is_ping) {
		mute.unlock();
		return;
	}

	one.ping = system_clock::now();
	HASHCPY(one.hash, hash);
	one.is_node = is_node;
	one.ipp = ipp;

	structs::clients.push_back(one);
	mute.unlock();

	if (is_node) {
		storage::nodes.add(hash, ipp.ip);
	}
}
/*********************************************************/
void clients_handler::rm_ip(string ip) {
	using structs::clients;

	vector<struct client>::iterator it;

	if (ip.length() < 8) {
		return;
	}

	mute.lock();
	it = clients.begin();

	for (; it != clients.end(); it++) {
		if ((*it).ipp.ip != ip) {
			continue;
		}

		clients.erase(it);
		break;
	}

	mute.unlock();
}
/*********************************************************/
void clients_handler::rm_hash(unsigned char *hash) {
	using structs::clients;

	vector<struct client>::iterator it;

	if (!hash) {
		return;
	}

	mute.lock();
	it = clients.begin();

	for (; it != clients.end(); it++) {
		if (memcmp((*it).hash, hash, HASHSIZE) != 0) {
			continue;
		}

		clients.erase(it);
		break;
	}

	mute.unlock();
}
/*********************************************************/
void clients_handler::check(void) {
	using structs::clients;

	auto time = system_clock::now();
	size_t size;

	mute.lock();

	if ((size = clients.size()) == 0) {
		mute.unlock();
		return;
	}

	for (size_t i = 0; i < size; i++) {
		if (time - clients[i].ping < 8s) {
			continue;
		}

		clients.erase(clients.begin() + i);
	}

	mute.unlock();
}
/*********************************************************/
bool clients_handler::exists(unsigned char *hash) {
	bool status = false;

	mute.lock();

	for (auto &p : structs::clients) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		status = true;
		break;
	}

	mute.unlock();
	return status;
}
/*********************************************************/
bool clients_handler::find(unsigned char *hash,
						   struct client &one) {
	bool status = false;

	mute.lock();

	for (auto &p : structs::clients) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		status = true;
		one = p;
		break;
	}

	mute.unlock();
	return status;
}
/*********************************************************/
struct client clients_handler::nearest(unsigned char *hash) {
	struct client node = structs::father.info;
	int cmp;

	mute.lock();

	for (auto &p : structs::clients) {
		cmp = storage::father.cmp(hash, p.hash, node.hash);

		if (p.is_node && cmp == 1) {
			node = p;
			continue;
		}
	}

	mute.unlock();
	return node;
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void clients_handler::print(void) {
	cout << "Hash, Ip:Port, Is node" << endl
		 << "----------------------" << endl;
	mute.lock();

	for (auto &p : structs::clients) {
		cout << bin2hex(HASHSIZE, p.hash) << ", " 
			 << p.ipp.ip << ":" << p.ipp.port
			 << ((p.is_node) ? ", True" : ", False")
			 << endl;
	}

	mute.unlock();
	cout << endl << endl;
}

#endif