// if not tcp_sender - 
#include "../../include/storage.hpp"
/*********************************************************/
void tunnels_handler::user_create(unsigned char *hash,
								  enum tcp_role role,
								  size_t code,
								  struct haship hi) {
	using structs::tunnels;
	using structs::father;
	using storage::nodes;

	vector<struct tunnel *>::iterator it;
	struct tunnel *tunn;
	size_t id;

	assert(hash);

	if (exists_u(hash)) {
		return;
	}

	auto cmp = [](struct tunnel *tunn, size_t code) {
		return tunn->code < code;
	};

	assert(tunn = new tunnel());

	tunn->code = (role == TCP_SENDER) ? random_cookie() : code;
	tunn->id = id = byte_sum(hash, HASHSIZE);
	tunn->time = system_clock::now();
	tunn->node = nodes.get_notdad();
	tunn->binder1 = hi;
	tunn->role = role;

	HASHCPY(tunn->hash, hash);

	mute.lock();

	it = lower_bound(tunnels.begin(), tunnels.end(), id, cmp);
	tunnels.insert(it, tunn);
	
	mute.unlock();

	port_req(tunn->node.hash, tunn->node.ipp);
}
/*********************************************************/
void tunnels_handler::node_create(struct client user,
								  enum tcp_role role,
								  size_t code,
								  struct haship hi) {
	using structs::tunnels;

	vector<struct tunnel *>::iterator it;
	struct tunnel *tunn;

	if (code == 0 || exists_n(role, code)) {
		return;
	}

	assert(tunn = new tunnel());

	auto cmp = [](struct tunnel *tunn, size_t code) {
		return tunn->code < code;
	};

	tunn->time = system_clock::now();
	tunn->role = role;
	tunn->user = user;
	tunn->code = code;
	tunn->binder1 = hi;
	HASHCPY(tunn->node.hash, hi.hash);
	tunn->node.ipp.ip = hi.ip;

	mute.lock();

	it = lower_bound(tunnels.begin(), tunnels.end(), code,
					 cmp);
	tunnels.insert(it, tunn);

	mute.unlock();

	if (role == TCP_BINDER2) {
		port_req(hi.hash, {UDP_PORT, hi.ip});
	}
}
/*********************************************************/
bool tunnels_handler::find_hash(unsigned char *hash,
								struct tunnel **one) {
	using structs::tunnels;

	vector<struct tunnel *>::iterator it;
	bool status = false;
	size_t id;

	assert(hash);
	assert((id = byte_sum(hash, HASHSIZE)) != 0);

	auto cmp = [](struct tunnel *tunn, size_t id) {
		return tunn->id < id;
	};

	mute.lock();
	it = tunnels.begin();

	for (;; it++) {
		it = lower_bound(it, tunnels.end(), id, cmp);

		if (it == tunnels.end() || (*it)->id != id) {
			break;
		}

		if (HASHEQ((*it)->hash, hash)) {
			status = true; *one = *it;
			break;
		}
	}

	mute.unlock();

	return status;
}
/*********************************************************/
bool tunnels_handler::find_code(size_t code, enum tcp_role r,
								struct tunnel **one) {
	using structs::tunnels;
	
	vector<struct tunnel *>::iterator it;
	bool status = false;

	assert(code != 0 && r != TCP_NONE);

	auto cmp = [](struct tunnel *tunn, size_t code) {
		return tunn->code < code;
	};

	mute.lock();
	it = tunnels.begin();

	for (;; it++) {
		it = lower_bound(it, tunnels.end(), code, cmp);

		if (it == tunnels.end() || (*it)->code != code) {
			break;
		}

		if ((*it)->role == r) {
			status = true; *one = *it;
			break;
		}
	}

	mute.unlock();

	return status;		
}
/*********************************************************/
void tunnels_handler::port_req(unsigned char *hash,
							   struct ipport ipp) {
	req_port req;
	size_t port;
	pack msg;

	assert(hash);

	port = storage::ports.try_reg(hash);
	req.port(port);
	msg.from(req);

	storage::tasks.add(msg, sddr_get(ipp), hash);
}
/*********************************************************/
bool tunnels_handler::exists_n(enum tcp_role role,
							   size_t code) {
	struct tunnel *res;
	return find_code(code, role, &res);
} 
/*********************************************************/
bool tunnels_handler::exists_u(unsigned char *hash) {
	struct tunnel *res;

	assert(hash);
	return find_hash(hash, &res);
}
/*********************************************************/
void tunnels_handler::sync(struct tunnel *tunn) {
	using storage::nodes;
	using storage::tasks;

	bool b = tunn->role == TCP_BINDER2;
	bool s = tunn->role == TCP_SENDER;
	bool t = tunn->role == TCP_TARGET;

	struct tcp_port p;
	req_tunnel req;
	pack msg;

	bool ext = storage::ports.find(tunn->node.hash, p);

	if (!ext && (s || t)) {
		tunn->node = nodes.get_notdad(tunn->node.hash);
		tunn->time = system_clock::now();

		port_req(tunn->node.hash, tunn->node.ipp);
	}

	if (!ext || !p.confirm) {
		return;
	}

	tunn->tcp.start(s, p.port);
	tunn->time = system_clock::now();

	storage::ports.rm_port(p.port);

	req.role(tunn->role);
	req.code(tunn->code);

	if (t) {
		req.hash(tunn->binder1.hash);
		req.ipaddr(tunn->binder1.ip);
	}

	if (s) {
		msg_request(tunn);
	}

	tunn->node.ipp.port = UDP_PORT;
	tunn->node.ipp.ip = (b) ? tunn->node.ipp.ip
							: tunn->binder1.ip;
	msg.from(req);

	tasks.add(msg, sddr_get(tunn->node.ipp),
			 (b) ? tunn->binder1.hash : tunn->node.hash);
}
/*********************************************************/
void tunnels_handler::msg_request(struct tunnel *tunn) {
	using structs::father;
	using storage::tasks;
	using storage::routes;

	struct haship hi;
	struct route to;
	req_msg req;
	pack msg;

	if (!routes.find(tunn->hash, to) || to.status != FOUND) {
		return;
	}

	HASHCPY(hi.hash, to.node);
	hi.ip = to.ipp.ip;

	req.ipaddr(tunn->node.ipp.ip, tunn->hash);
	req.from(structs::keys.pub, tunn->hash);
	req.hash(tunn->node.hash, tunn->hash);
	req.code(tunn->code, tunn->hash);
	req.resend(hi);

	msg.from(req);

	tasks.add(msg, sddr_get(father.info.ipp), father.info.hash);
}
/*********************************************************/
void tunnels_handler::check(void) {
	using structs::tunnels;

	auto time = system_clock::now();
	struct tunnel *tunn;

	mute.lock();
	size_t size = tunnels.size();

	for (size_t i = 0; i < size; i++) {
		tunn = tunnels[i];

		if (tunn->role != TCP_BINDER1 && !tunn->sync()) {
			sync(tunn);
		}

		if (tunn->sync() && !tunn->tcp.work) {
			tunnels.erase(tunnels.begin() + i);
			delete tunn;
			continue;
		}

		if (!tunn->sync() && time - tunn->time > 20s) {
			tunnels.erase(tunnels.begin() + i);
			delete tunn;
			continue;
		}
	}

	mute.unlock();
}