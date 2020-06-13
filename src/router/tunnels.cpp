// hash
// hash to - hash from - port - status byte - is_error with routins

// добавить hash от кого запрос
#include "../../include/network.hpp"
/*********************************************************/
struct ret tunnels_router::req(struct sockaddr_in sddr,
							   pack msg) {
	unsigned char *to = msg.info(), *from = to + HASHSIZE;
	vector<struct route>::iterator it;
	pair<size_t, size_t> ports;
	bool exists1, exists2;
	struct route gate;
	pack res;

	res.tmp(NODE_RES_TUNNEL, msg.cookie());
	res.set_info(to, HASHSIZE * 2);

	if (is_null(to, HASHSIZE * 2)) {
		return handler::make_ret(sddr, msg.hash(), res);
	}

	if (storage::tunnels.find_ports(to, from, ports)) {
		res.add_info(HASHSIZE * 2, &ports.second, 4);
		return handler::make_ret(sddr, msg.hash(), res);
	}
	/**
	*	If the request is for us.
	*/
	if (IS_ME(to)) {
		if (!storage::father.from_father(sddr, msg)) {
			return handler::make_ret(sddr, nullptr,
									 res);
		}
		return this->get_message(sddr, to, msg.hash(),
								 msg.cookie());
	}

	struct client client;
	enum tcp_role role;

	exists1 = storage::clients.find(to, client);
	exists2 = storage::routes.find(to, it);
	// Если нету маршрута, сначала нужно сделать поиск!!
	if (!exists1 && !exists2) {
		return handler::make_ret(sddr, msg.hash(), res);
	}

	role = (*(from + HASHSIZE + 4) == 0x00) ? TCP_UBINDER 
											: TCP_BINDER2;
	gate = *it;

	if (exists1) {
		HASHCPY(gate.father, client.hash);
		//HASHCPY(gate.hash, client.hash);
		gate.ipp = client.ipp;
	}

	return this->create_tunnel((exists2) ? TCP_BINDER1 : role,
						sddr, gate, msg);
}
/*********************************************************/
struct ret tunnels_router::get_message(struct sockaddr_in sddr,
									   unsigned char *info,
									   unsigned char *hash,
									   size_t cookie) {
	using structs::role;

	struct init_tunnel init;
	size_t port;
	pack res;

	memcpy(&port, info + HASHSIZE * 2, 4);

	if (!storage::tunnels.is_freeport(port)) {
		/**
		*	If the port is busy, we don’t respond, maybe
		*	after resending it it will be free.
		*/
		return handler::make_ret(sddr, nullptr, res);
	}

	storage::inbox.add(info, port);

	res.tmp((role == UDP_NODE) ? NODE_RES_TUNNEL
							   : USER_RES_TUNNEL, cookie);

	return handler::make_ret(sddr, hash, res);
}
/*********************************************************/
struct ret tunnels_router::create_tunnel(enum tcp_role role,
										 struct sockaddr_in sddr,
										 struct route gate,
										 pack msg) {
	using storage::routes;
	using storage::tasks;

	unsigned char *to = msg.info(), *from = to + HASHSIZE;
	size_t shift = HASHSIZE * 2;
	struct sockaddr_in rsddr;
	struct init_tunnel init;
	size_t port;
	pack req, res;

	res.tmp(NODE_RES_TUNNEL, msg.cookie());
	res.set_info(to, shift);
	/**
	*	Add a return route (This will help not to look for a
	*	client, for a return answer).
	*/
	if (role == TCP_BINDER2) {
		routes.set(true, from, msg.hash(), ipport_get(sddr));
	}

	port = storage::tunnels.free_port();
	rsddr = sddr_get(gate.ipp);

	if (role != TCP_BINDER1) {
		init = {TCP_SND, to, role, {port, gate.ipp.ip}};
		storage::tunnels.add(to, from, init);

		req.tmp(NODE_REQ_TUNNEL);
		req.set_info(msg.info(), shift);
		/**
		*	In the information to the end user, indicate the
		*	port on which you can receive the message.
		*/
		req.add_info(shift, &port, 4);
		tasks.add(req, rsddr,  to);
		port = storage::tunnels.free_port();
	}

	init = {TCP_RCV, gate.father, role, {port, ""}};
	storage::tunnels.add(to, from, init);

	res.add_info(shift, &port, 4);
	tasks.add(res, sddr, msg.hash());

	req.tmp(NODE_REQ_TUNNEL);
	req.set_info(msg.info(), shift);
	/**
	*	Set the flag that the next node is the second binder.
	*/
	unsigned char flag = (role == TCP_BINDER1) ? 0x01
											   : 0x00;
	req.add_info(shift + 4, &flag, 1);
	return handler::make_ret(sddr, msg.hash(), req);
}
/*********************************************************/
void tunnels_router::res(struct sockaddr_in sddr, pack msg) {
	unsigned char *to = msg.info(), *from = to + HASHSIZE;
	vector<struct route>::iterator it;
	struct init_tunnel init;
	enum tcp_role role;
	size_t port;

	storage::tasks.rm_cookie(msg.cookie());
	memcpy(&port, from + HASHSIZE, 4);

	if (port == 0) {
		/**
		*	If the route has not been made, the port value
		*	will be 0. Ignore the answer.
		*/
		return;
	}

	role = (*(from + HASHSIZE + 4) == 0x01) ? TCP_BINDER2
											: TCP_BINDER1;
	/**
	*	Игнорируем ответ от конечного клиента.
	*/
	if (!storage::tunnels.is_freeport(port)
		|| role == TCP_BINDER2) {
		return;
	}

	init = {TCP_SND, msg.hash(), role, {port, ""}};

	if (storage::routes.find(to, it)) {
		init.ipp.ip = it->ipp.ip;
		storage::tunnels.add(to, from, init);
		return;
	}
	/**
	*	We are the initiator of the request - we create a
	*	stream to send a message.
	*/
	init.ipp.ip = ipport_get(sddr).ip;
	init.role = TCP_SENDER;

	storage::tunnels.add(to, from, init);
}