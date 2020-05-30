
#include "../../include/storage.hpp"
/*********************************************************/
nodes_handler::nodes_handler(void) {
	ping = system_clock::now();
}
/*********************************************************/
void nodes_handler::select(void) {
	map<unsigned char *, string> list = storage::db.nodes();
	struct client one;

	mute.lock();

	for (auto &p : list) {
		assert(p.first);

		memcpy(one.hash, p.first, HASHSIZE);
		one.ipp.ip = p.second;
		one.ipp.port = UDP_PORT;
		delete[] p.first;

		structs::nodes.push_back(one);
	}

	mute.unlock();

	if (list.size() == 0) {
		cout << "[E]: Node list is empty.\n";
		exit(1);
	}
}
/*********************************************************/
void nodes_handler::temporary_father(void) {
	using storage::father;
	using structs::keys;

	struct client dad;

	mute.lock();

	if (structs::nodes.size() == 0) {
		cout << "[E]: Node list is empty.\n";
		exit(1);
	}

	dad = structs::nodes[0];

	for (auto &p : structs::nodes) {
		if (father.cmp(keys.pub, dad.hash, p.hash) == 1) {
			continue;
		}

		dad = p;
	}

	mute.unlock();
	storage::father.set(dad);
}
/*********************************************************/
void nodes_handler::rm_hash(unsigned char *hash) {
	vector<struct client>::iterator it;
	string ip;

	assert(hash);

	mute.lock();
	it = structs::nodes.begin();

	for (; it != structs::nodes.end(); it++) {
		if (memcmp((*it).hash, hash, HASHSIZE) != 0) {
			continue;
		}

		ip = (*it).ipp.ip;
		structs::nodes.erase(it);
		break;
	}

	mute.unlock();
	storage::db.rm_node(ip);
}
/*********************************************************/
void nodes_handler::add(unsigned char *hash, string ip) {
	struct client one;

	assert(hash && ip.length() > 5);
	mute.lock();

	if (structs::nodes.size() >= NODE_LIMIT) {
		mute.unlock();
		return;
	}

	for (auto p : structs::nodes) {
		if (memcmp(hash, p.hash, HASHSIZE) == 0) {
			mute.unlock();
			return;
		}
	}

	memcpy(one.hash, hash, HASHSIZE);
	one.ipp = { UDP_PORT, ip };

	structs::nodes.push_back(one);
	mute.unlock();

	storage::db.rm_node(ip);
	storage::db.add_node(one.hash, ip);
}
/*********************************************************/
void nodes_handler::check(void) {
	using structs::father;
	using structs::nodes;
	using structs::role;

	auto time = system_clock::now() - ping;
	pack msg;

	mute.lock();

	if (nodes.size() >= NODE_MIN || time > 500s
		|| role == UDP_NONE) {
		mute.unlock();
		return;
	}

	ping = system_clock::now();
	mute.unlock();

	if (role == UDP_NODE) {
		this->from_clients();
		return;
	}

	if (structs::nodes.size() >= NODE_MIN) {
		return;
	}

	msg.tmp(USER_REQ_NODE);
	storage::tasks.add(msg, sddr_get(father.info.ipp),
	                   father.info.hash);
}
/*********************************************************/
void nodes_handler::from_clients(void) {
	storage::clients.mute.lock();

	for (auto p : structs::clients) {
		if (structs::nodes.size() >= NODE_MIN) {
			break;
		}

		if (!p.is_node) {
			continue;
		}

		this->add(p.hash, p.ipp.ip);
	}

	storage::clients.mute.unlock();
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void nodes_handler::print(void) {
	cout << "Hash, Ip:Port" << endl
		 << "----------------------" << endl;
	mute.lock();

	for (auto &p : structs::nodes) {
		cout << bin2hex(HASHSIZE, p.hash) << ", " 
			 << p.ipp.ip << ":" << p.ipp.port
			 << endl;
	}

	mute.unlock();
	cout << endl << endl;
}

#endif