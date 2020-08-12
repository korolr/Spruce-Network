
#include "../../include/storage.hpp"

void routes_handler::add(unsigned char *hash, size_t code,
						 enum route_state state) {
	vector<struct route>::iterator it;
	auto time = system_clock::now();
	bool status = false;
	struct route one;

	assert(hash);

	one.id = byte_sum(hash, HASHSIZE);
	HASHCPY(one.hash, hash);
	one.status = state;
	one.time = time;
	one.code = code;

	mute.lock();

	status = vector_search(it, structs::routes, hash);

	if (status && time - it->time <= 60s) {
		it->status = state;
		mute.unlock();
		return;
	}

	if (status) {
		structs::routes.erase(it);
	}

	vector_push(structs::routes, one);
	mute.unlock();
}

bool routes_handler::upd(unsigned char *hash,
						 size_t code,
						 struct haship node) {
	vector<struct route>::iterator it;

	assert(!IS_NULL(node.hash, HASHSIZE) && hash
			&& node.ip.length() > 0);

	mute.lock();

	if (!vector_search(it, structs::routes, hash)) {
		mute.unlock();
		return false;
	}

	it->time = system_clock::now();

	if (it->code != code) {
		it->status = NFOUND;
		mute.unlock();
		return true;
	}

	it->ipp = { UDP_PORT, node.ip};
	HASHCPY(it->node, node.hash);
	it->status = FOUND;

	mute.unlock();
	return true;
}

bool routes_handler::find(unsigned char *hash,
						  struct route &one) {
	vector<struct route>::iterator it;

	assert(hash);
	mute.lock();

	if (!vector_search(it, structs::routes, hash)) {
		mute.unlock();
		return false;
	}

	one = *it;
	mute.unlock();

	return true;
}

void routes_handler::ping(unsigned char *hash) {
	vector<struct route>::iterator it;

	assert(hash);
	mute.lock();

	if (vector_search(it, structs::routes, hash)) {
		it->time = system_clock::now();
	}

	mute.unlock();
}

void routes_handler::check(void) {
	using structs::routes;

	mute.lock();

	auto rmc = remove_if(routes.begin(), routes.end(),
						 [&](struct route &element) {
		auto time = system_clock::now();

		if (time - element.time <= 60s) {
			return false;
		}

		if (element.status == PROGRESS) {
			element.status = NFOUND;
			element.time = time;
			return false;
		}

		return time - element.time > 100s;
	});

	routes.erase(rmc, routes.end());
	mute.unlock();
}