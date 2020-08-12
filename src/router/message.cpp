
#include "../../include/network.hpp"

struct ret message_router::request(struct sockaddr_in sddr,
								   pack msg) {
	struct ipport ipp;
	struct haship hi;
	pack chain, back;
	req_msg req;

	back.tmp((structs::role == UDP_USER) ? USER_REQ_MSG
										 : NODE_REQ_MSG, 
										 msg.cookie());
	msg.to(req);

	if (structs::role == UDP_USER) {
		message_processing(req);
		return handler::make_ret(sddr, msg.hash(), back);
	}

	hi = req.resend();

	if (hi.ip.length() > 0 && !IS_NULL(hi.hash, HASHSIZE)) {
		ipp.port = UDP_PORT;
		ipp.ip = hi.ip;

		req.resend({ }, true);
		chain.from(req);
		chain.change_cookie();

		storage::tasks.add(chain, sddr_get(ipp), hi.hash);
		return  handler::make_ret(sddr, msg.hash(), back);
	}

	auto list = storage::clients.users();

	for (auto &p : list) {
		chain.from(req);
		chain.change_cookie();

		storage::tasks.add(chain, sddr_get(p.ipp), p.hash);
	}

	return handler::make_ret(sddr, msg.hash(), back);
}

void message_router::message_processing(req_msg &req) {
	struct haship hash_ip;

	if (!req.decode) {
		return;
	}

	HASHCPY(hash_ip.hash, req.hash());
	storage::routes.ping(req.from());
	hash_ip.ip = req.ipaddr();

	storage::tunnels.user_create(req.from(), TCP_TARGET,
								 req.code(), hash_ip);
	// User thread for data receiving. Add data to inbox;

}