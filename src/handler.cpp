
#include "../include/network.hpp"
/*********************************************************/
void handler::node(pack msg, struct sockaddr_in sddr) {
	unsigned char role = UDP_NODE;
	bool ff = false;
	struct ret fres;
	pack res;

	if ((ff = storage::father.from_father(sddr, msg))) {
		structs::father.info.ping = system_clock::now();
	}

	if (queue.lock(msg)) {
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
		storage::tasks.rm_cookie(msg.cookie());
		break;

	case REQ_ROLE:
		res.tmp(RES_ROLE, msg.cookie());                    cout << "REQ_ROLE\n";

		if (ipport_get(sddr).port == UDP_PORT) {
			res.set_info(&role, 1);
		}
		else {
			role = UDP_USER;
			res.set_info(&role, 1);
		}

		fres = make_ret(sddr, msg.hash(), res);
		break;

	case NODE_REQ_FATHER:
	case USER_REQ_FATHER:
		fres = router::father.req(sddr, msg);                     //cout << "REQ_FATHER\n";
		break;

	case NODE_RES_FATHER:
		router::father.res(sddr, msg);                           //cout << "RES_FATHER\n";
		break;

	case USER_REQ_FIND:
	case NODE_REQ_FIND:
																cout << "REQ_FIND\n";
		fres = router::find.req(sddr, msg);
		break;

	case USER_REQ_NODE:
	case NODE_REQ_NODE:
		fres = router::nodes.req(sddr, msg);                    cout << "REQ_NODE\n";
		break;

	case NODE_RES_NODE:
		router::nodes.res(sddr, msg);                           cout << "RES_NODE\n";
		break;

	case USER_REQ_TUNNEL:
	case NODE_REQ_TUNNEL:
		fres = router::tunnels.req(sddr, msg);                  cout << "REQ_TUNNEL\n";
		break;

	case NODE_RES_TUNNEL:
		router::tunnels.res(sddr, msg);                         cout << "RES_TUNNEL\n";
		break;

	case NODE_RES_FIND:
		router::find.res(sddr, msg);                            cout << "RES_FIND\n";
		break;

	default:
		if (ff) {
			storage::father.no_father();
		}
	}

	if (!fres.empty) {
		storage::tasks.add(fres.msg, fres.sddr,
						   fres.hash);
	}

	queue.unlock(msg);
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

	case NODE_RES_FIND:
		router::find.res(sddr, msg);
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