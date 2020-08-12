
#include "../include/network.hpp"
/*********************************************************/
void handler::node(pack msg, struct sockaddr_in sddr) {
	struct ipport ipp = ipport_get(sddr);
	unsigned char role;
	bool ff = false;
	struct ret fres;
	pack res;

	if (!structs::father.status && msg.type() != RES_FATHER) {
		res.tmp(DOS, msg.cookie());
		storage::tasks.add(res, sddr, msg.hash());
		return;
	}

	if ((ff = storage::father.from_father(sddr, msg))) {
		structs::father.info.ping = system_clock::now();
	}

	if (queue.lock(msg, ipp.ip)) {
		return;
	}

	fres.empty = true;

	switch (msg.type()) {
	case USER_REQ_PING:
	case NODE_REQ_PING:
		res.tmp(NODE_RES_PING,   msg.cookie());
		fres = make_ret(sddr, msg.hash(), res);
		break;

	case NODE_RES_PING:
	case NODE_RES_FIND:
	case NODE_RES_MSG:
		storage::tasks.rm_cookie(msg.cookie());
		break;

	case REQ_ROLE:
		res.tmp(RES_ROLE, msg.cookie());
		role = (ipp.port != UDP_PORT) ? UDP_USER
									  : UDP_NODE;
		res.set(&role, 1);
		fres = make_ret(sddr, msg.hash(), res);
		break;

	case REQ_FATHER:
		fres = router::father.request(sddr, msg);
		break;

	case USER_REQ_PORT:
	case NODE_REQ_PORT:
		fres = router::port.request(sddr, msg);
		break;

	case USER_REQ_FIND:
	case NODE_REQ_FIND:
		fres = router::find.request(sddr, msg);
		break;

	case USER_REQ_MSG:
	case NODE_REQ_MSG:
		fres = router::message.request(sddr, msg);
		break;

	case USER_RES_PORT:
	case NODE_RES_PORT:
		router::port.response(sddr, msg);
		break;

	case RES_FATHER:
		router::father.response(sddr, msg);
		break;

	default: break;
	}

	if (!fres.empty) {
		storage::tasks.add(fres.msg, fres.sddr,
						   fres.hash);
	}

	queue.unlock(msg, ipp.ip);
}
/*********************************************************/
void handler::user(pack msg, struct sockaddr_in sddr) {
	struct ret fres;
	bool ff;

	if ((ff = storage::father.from_father(sddr, msg))) {
		structs::father.info.ping = system_clock::now();
	}

	storage::nodes.sub_attempts(msg.hash());
	fres.empty = true;

	switch (msg.type()) {
	case NODE_RES_FIND:
	case NODE_RES_MSG:
		if (ff) {
			storage::tasks.rm_cookie(msg.cookie());
		}
		break;

	case RES_FATHER:
		router::father.response(sddr, msg);
		break;

	case NODE_REQ_FIND:
		if (ff) {
			fres = router::find.request(sddr, msg);
		}
		break;

	case NODE_REQ_PORT:
		fres = router::port.request(sddr, msg);
		break;

	case NODE_REQ_MSG:
		if (ff) {
			fres = router::message.request(sddr, msg);
		}
		break;

	default: break;
	}

	if (!fres.empty) {
		storage::tasks.add(fres.msg, fres.sddr,
						   fres.hash);
	}
}
/*********************************************************/
struct ret handler::make_ret(struct sockaddr_in sddr,
							 unsigned char *hash,
							 pack msg) {
	struct ret data;

	data.empty	= (!hash) ? true : false;
	data.sddr	= sddr;
	data.msg	= msg;

	if (hash) {
		HASHCPY(data.hash, hash);
	}

	return data;
}