
#include "../../include/network.hpp"

struct ret find_router::request(struct sockaddr_in sddr,
								pack msg) {
	using handler::make_ret;
	using storage::father;
	using structs::role;

	pack back;

	back.tmp((role == UDP_USER) ? USER_RES_FIND
								: NODE_RES_FIND,
								msg.cookie());

	if (role == UDP_USER) {
		find_processing(sddr, msg);
		return make_ret(sddr, msg.hash(), back);
	}

	if (!allow(sddr, msg.info())) {
		return make_ret(sddr, msg.hash(), back);
	}

	unsigned char *publickey = structs::keys.pub;
	struct client near;
	req_find req;
	pack chain;

	msg.to(req);
	near = storage::clients.nearest(req.who());

	if (father.cmp(req.who(), publickey, near.hash) != 2) {
		packet_distribution(req);
		return make_ret(sddr, msg.hash(), back);
	}

	chain.from(req);
	chain.change_cookie();

	storage::tasks.add(chain, sddr_get(near.ipp), near.hash);
	return make_ret(sddr, msg.hash(), back);
}


void find_router::find_processing(struct sockaddr_in sddr,
								  pack msg) {
	unsigned char from[HASHSIZE];
	req_find req;
	pack back;

	msg.to(req);

	if (!req.decode || !HASHEQ(req.who(), structs::keys.pub)) {
		return;
	}

	HASHCPY(from, req.from());

	if (storage::routes.upd(from, req.code(), req.father()))  {
		return;
	}

	req.father(storage::father.haship(), from);
	req.from(structs::keys.pub, from);
	req.code(req.code(), from);
	req.who(from);

	back.from(req);
	back.change_cookie();

	storage::tasks.add(back, sddr, msg.hash());
	storage::routes.add(from, 0, FOUND);
}

void find_router::packet_distribution(req_find req) {
	auto list = storage::clients.users();
	pack user;

	if (list.empty()) {
		return;
	}

	user.from(req);

	for (auto &p : list)   {
		user.change_cookie();
		storage::tasks.add(user, sddr_get(p.ipp), p.hash);
	}
}

bool find_router::allow(struct sockaddr_in sddr,
						unsigned char *hash) {
	string ip = ipport_get(sddr).ip;
	struct freqs one;

	if (IS_NULL(hash, HASHSIZE)) {
		return false;
	}

	mute.lock();

	auto r = remove_if(reqs.begin(), reqs.end(),
					   [](struct freqs &el) {
		return system_clock::now() - el.time > 15s;
	});

	reqs.erase(r, reqs.end());

	for (auto &p : reqs) {
		if (p.ip == ip && HASHEQ(hash, p.hash)) {
			mute.unlock();
			return false;
		}
	}

	one.time = system_clock::now();
	HASHCPY(one.hash, hash);
	one.ip = ip;

	reqs.push_back(one);
	mute.unlock();

	return true;
}
