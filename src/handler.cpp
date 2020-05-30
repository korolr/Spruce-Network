
#include "../include/network.hpp"
/*********************************************************/
void handler::node(pack msg, struct sockaddr_in sddr) {
	struct ipport ipp = ipport_get(sddr);
	unsigned char role = UDP_NODE;
	bool ff = false;
	struct ret resp;
	pack response;

	if ((ff = storage::father.from_father(sddr, msg))) {
		structs::father.info.ping = system_clock::now();
	}

	resp.empty = true;

	switch (msg.type()) {
	case USER_REQ_PING:
	case NODE_REQ_PING:
		response.tmp(NODE_RES_PING, msg.cookie());
		storage::tasks.add(response, sddr, msg.hash());
		return;

	case NODE_RES_PING:
		return;

	case REQ_ROLE:
		response.tmp(RES_ROLE, msg.cookie());

		if (ipp.port == UDP_PORT) {
			response.set_info(&role, 1);
		}
		else {
			role = UDP_USER;
			response.set_info(&role, 1);
		}

		storage::tasks.add(response, sddr, msg.hash());
		return;

	case NODE_REQ_FATHER:
	case USER_REQ_FATHER:
		resp = router::father.req(sddr, msg);
		break;

	case NODE_RES_FATHER:
		router::father.res(sddr, msg);
		break;

	case NODE_REQ_FIND:
		resp = router::find.req(sddr, msg);
		break;

	case USER_REQ_NODE:
	case NODE_REQ_NODE:
		resp = router::nodes.req(sddr, msg);
		break;

	case NODE_RES_NODE:
		router::nodes.res(sddr, msg);
		break;

	case USER_REQ_TUNNEL:
	case NODE_REQ_TUNNEL:
		resp = router::tunnels.req(sddr, msg);
		break;

	case NODE_RES_TUNNEL:
		router::tunnels.res(sddr, msg);
		break;

	case NODE_RES_FIND:
		router::find.res(sddr, msg);
		break;

	default:
		if (ff) {
			storage::father.no_father();
			return;
		}

		response.tmp(RES_ROLE, msg.cookie());
		storage::tasks.add(response, sddr, msg.hash());
		return;
	}

	if (!resp.empty) {
		storage::tasks.add(resp.msg, resp.sddr, resp.hash);
	}
}
/*********************************************************/
void handler::user(pack msg, struct sockaddr_in sddr) {
	if (!storage::father.from_father(sddr, msg)) {
		return;
	}

	structs::father.info.ping = system_clock::now();

	switch (msg.type()) {
	case NODE_RES_PING:
		storage::tasks.rm_cookie(msg.cookie());
		return;

	case NODE_RES_FATHER:
		router::father.res(sddr, msg);
		return;

	case NODE_RES_NODE:
		router::nodes.res(sddr, msg);
		return;

	case NODE_RES_TUNNEL:
		router::tunnels.res(sddr, msg);
		return;

	default:
		storage::father.no_father();
	}
}
/*********************************************************/
void handler::role(pack msg, struct sockaddr_in sddr) {
	using structs::father;

	unsigned char *role = msg.info();
	enum udp_type type;
	pack dad_req;

	structs::role = (*role == UDP_NODE) ? UDP_NODE
										: UDP_USER;

	type = (structs::role == UDP_USER)  ? USER_REQ_FATHER
										: NODE_REQ_FATHER;
	if (storage::tasks.exists(type)) {
		return;
	}

	dad_req.tmp(type);
	storage::tasks.add(dad_req, sddr_get(father.info.ipp),
					   father.info.hash);
	father.info.ping = system_clock::now();
}
/*********************************************************/
struct ret handler::make_ret(struct sockaddr_in sddr,
							 unsigned char *hash,
							 pack msg) {
	/*struct ret data = {	.empty	= (!hash) ? true : false, 
						.sddr	= sddr,
						.msg	= msg
	};*/
	struct ret data;

	data.empty	= (!hash) ? true : false;
	data.sddr	= sddr;
	data.msg	= msg;

	if (hash) {
		memcpy(data.hash, hash, HASHSIZE);
	}

	return data;
}