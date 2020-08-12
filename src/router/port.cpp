
#include "../../include/network.hpp"

struct ret port_router::request(struct sockaddr_in sddr,
								pack msg) {
	req_port req;
	res_port res;
	pack back;

	msg.to(req);

	bool r = storage::ports.reg(req.port(), msg.hash());
	res.cookie = msg.cookie();
	res.port(req.port());
	res.accepted(r, true);

	back.from(res);

	return handler::make_ret(sddr, msg.hash(), back);
}

void port_router::response(struct sockaddr_in sddr,
						   pack msg) {
	req_port req;
	res_port res;
	size_t port;
	pack back;

	storage::tasks.rm_cookie(msg.cookie());
	msg.to(res);

	if (res.accepted()) {
		storage::ports.reg(res.port(), msg.hash());
		return;
	}

	port = storage::ports.try_reg(msg.hash());
	req.port(port);

	back.from(req);

	storage::tasks.add(msg, sddr, msg.hash());
}