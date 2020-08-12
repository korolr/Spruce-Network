
#include "../../include/storage.hpp"

/*********************************************************/
void nodes_handler::select(void) {
	using structs::nodes;

	auto list = storage::db.nodes();
	struct client_att one;

	if (list.empty()) {
		cout << "[E]: Node list is empty.\n";
		exit(1);
	}

	for (auto &p : list) {
		assert(p.first);

		one.id = byte_sum(p.first, HASHSIZE);
		one.ipp = { UDP_PORT, p.second };
		one.ping = system_clock::now();
		HASHCPY(one.hash, p.first);

		delete[] p.first;
		vector_push(nodes, one);
	}
}
/*********************************************************/
size_t nodes_handler::size(void) {
	mute.lock();
	size_t size = structs::nodes.size();
	mute.unlock();

	return size;
}
/*********************************************************/
void nodes_handler::rm_hash(unsigned char *hash) {
	vector<struct client_att>::iterator it;
	string ip;

	assert(hash);

	mute.lock();

	if (!vector_search(it, structs::nodes, hash)) {
		mute.unlock();
		return;
	}

	ip = it->ipp.ip;
	structs::nodes.erase(it);

	mute.unlock();
	storage::db.rm_node(ip);
}
/*********************************************************/
void nodes_handler::add(unsigned char *hash, string ip) {
	using structs::nodes;

	vector<struct client_att>::iterator it;
	struct client_att one;

	assert(hash && ip.length() > 5);
	mute.lock();

	if (IS_ME(hash)) {
		mute.unlock();
		return;
	}

	if (vector_search(it, nodes, hash)) {
		it->ping = system_clock::now();
		it->attempts = 0;
		mute.unlock();
		return;
	}

	if (nodes.size() >= NODE_LIMIT) {
		storage::db.rm_node(nodes.begin()->ipp.ip);
		nodes.erase(nodes.begin());
	}

	one.id = byte_sum(hash, HASHSIZE);
	one.ipp = {UDP_PORT, ip};
	HASHCPY(one.hash, hash);
	vector_push(nodes, one);

	mute.unlock();

	storage::db.rm_node(ip);
	storage::db.add_node(hash, ip);
}
/*********************************************************/
void nodes_handler::check(void) {
	using structs::nodes;
return;
	mute.lock();

	if (structs::nodes.empty()) {
		cout << "[E]: Node list is empty. Please update"
			 << " the database(1).\n";
		exit(1);
	}

	auto r = remove_if(nodes.begin(), nodes.end(),
					   [](struct client_att one){
		auto time = system_clock::now();
		return one.attempts >= 3 && time - one.ping > 10s;
	});

	nodes.erase(r, nodes.end());
	mute.unlock();
}
/*********************************************************/
struct client nodes_handler::nearest(unsigned char *hash) {
	using storage::father;

	struct client_att node = structs::nodes[0];

	assert(hash);
	mute.lock();

	if (structs::nodes.empty()) {
		cout << "[E]: Node list is empty. Please update"
			 << " the database(2).\n";
		exit(1);
	}

	for (auto &p : structs::nodes) {
		if (IS_ME(p.hash)) {
			continue;
		}

		if (father.cmp(hash, p.hash, node.hash) == 1) {
			node = p;
		}
	}

	mute.unlock();

	return static_cast<struct client>(node);
}
/*********************************************************/
void nodes_handler::add_attempts(unsigned char *hash) {
	vector<struct client_att>::iterator it;
	auto time = system_clock::now();

	assert(hash);

	mute.lock();

	if (vector_search(it, structs::nodes, hash)
		&& time - it->ping > 9s) {
		it->attempts++;
		it->ping = time;
	}

	mute.unlock();
}
/*********************************************************/
void nodes_handler::sub_attempts(unsigned char *hash) {
	vector<struct client_att>::iterator it;

	assert(hash);

	mute.lock();

	if (vector_search(it, structs::nodes, hash)) {
		it->ping = system_clock::now();
		it->attempts = 0;
	}

	mute.unlock();
}
/*********************************************************/
struct client nodes_handler::get_notdad(unsigned char *hash) {
	vector<struct client_att>::iterator it;
	struct client ret;

	mute.lock();

	if (structs::nodes.size() < 3) {
		cout << "Can't get node which is note current"
			 << " father, please update the database.\n";
		exit(1);
	}

	it = structs::nodes.begin();

	if (it->ipp == structs::father.info.ipp) {
		it++;
	}

	if (hash && HASHEQ(it->hash, hash)) {
		it++;
	}

	ret = static_cast<struct client>(*it);
	mute.unlock();

	return ret;
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void nodes_handler::print(void) {
	cout << "Id, Hash, Ip:Port" << endl
		 << "----------------------" << endl;
	mute.lock();

	for (client_att &p : structs::nodes) {
		cout << p.id << ", "
			 << bin2hex(HASHSIZE, p.hash) << ", " 
			 << p.ipp.ip << ":" << p.ipp.port
			 << endl;
	}

	mute.unlock();
	cout << endl << endl;
}

#endif