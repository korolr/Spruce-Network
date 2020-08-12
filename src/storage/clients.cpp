
#include "../../include/storage.hpp"
/*********************************************************/
void clients_handler::reg(unsigned char *hash,
						  struct ipport ipp,
						  enum udp_role role,
						  bool is_ping) {
	struct client one;
	
	if (!hash || role == UDP_NONE || IS_ME(hash)) {
		return;
	}

	if (ipp.port == UDP_PORT && role == UDP_NODE) {
		one.is_node = true;
	}

	mute.lock();

	for (auto &p : structs::clients) {
		if (ipp != p.ipp) {
			continue;
		}

		p.ping = system_clock::now();
		HASHCPY(p.hash, hash);
		p.is_node = one.is_node;
		p.ipp = ipp;

		mute.unlock();
		return;
	}

	if (one.is_node) {
		storage::nodes.add(hash, ipp.ip);
	}

	if (!is_ping) {
		mute.unlock();
		return;
	}

	one.ping = system_clock::now();
	HASHCPY(one.hash, hash);
	one.ipp = ipp;

	structs::clients.push_back(one);
	mute.unlock();
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
		if (it->ipp.ip == ip) {
			clients.erase(it);
		}
	}

	mute.unlock();
}
/*********************************************************/
void clients_handler::check(void) {
	using structs::clients;

	mute.lock();

	auto rmc = [](struct client &one) {
		return system_clock::now() - one.ping > 8s;
	};

	clients.erase(remove_if(clients.begin(), clients.end(),
							rmc), clients.end());
	mute.unlock();
}
/*********************************************************/
struct client clients_handler::nearest(unsigned char *hash) {
	using storage::father;

	struct client node = structs::father.info;

	assert(hash);
	mute.lock();

	for (auto &p : structs::clients) {
		if (!p.is_node) {
			continue;
		}

		if (father.cmp(hash, p.hash, node.hash) == 1) {
			node = p;
			continue;
		}
	}

	mute.unlock();
	return node;
}
/*********************************************************/
vector<struct haship> clients_handler::veclist(void) {
	vector<struct haship> list;
	struct haship one;

	mute.lock();

	for (auto &p : structs::clients) {
		if (!p.is_node) {
			continue;
		}

		HASHCPY(one.hash, p.hash);
		one.ip = p.ipp.ip;

		list.push_back(one);
	}

	mute.unlock();

	return list;
}
/*********************************************************/
vector<struct client> clients_handler::users(void) {
	vector<struct client> list;

	mute.lock();

	for (auto &p : structs::clients) {
		if (!p.is_node) {
			list.push_back(p);
		}
	}

	mute.unlock();

	return list;
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