// hash
// hash to - hash from - port - status byte - is_error with routins

// добавить hash от кого запрос
#include "../../include/network.hpp"
/*********************************************************/
struct ret tunnels_router::req(struct sockaddr_in sddr,
							   pack msg) {
	using storage::father;
	using structs::keys;

	unsigned char *hash = msg.hash();
	unsigned char *to = msg.info();
	struct client nclient;
	enum tcp_role role;
	struct route fr;
	pack req, res;
	bool found;

	res.tmp(NODE_RES_TUNNEL, msg.cookie());
	res.set_info(to, HASHSIZE * 2);

	if (is_null(to, HASHSIZE * 2)) {
		return handler::make_ret(sddr, hash, res);
	}
	/**
	*	If the request is for us.
	*/
	if (IS_ME(to)) {
		return this->get_message(sddr, to, hash,
								 msg.cookie());
	}
	/**
	*	If the route has already been created before, we
	*	proceed to the generation of the TCP tunnel.
	*/
	if ((found = storage::routes.find(to, fr)) && fr.full) {
		/**
		*	If the fathe node is changed by user.
		*/
		if (!storage::clients.exists(to)) {
			unsigned char error  = 0x01;
			storage::routes.rm_hash(to);

			res.add_info(HASHSIZE * 2 + 5, &error, 1);
			return handler::make_ret(sddr, hash, res);
		}

		return this->newtunnel(sddr, fr, TCP_BINDER1, msg);
	}
	/**
	*	Network user in the search process.
	*/
	if (found && !fr.full) {
		return handler::make_ret(sddr, hash, res);
	}
	/**
	*	If the customer you are looking for is a registered
	*	user.
	*/
	if (storage::clients.find(to, nclient)) {
		HASHCPY(fr.father, to);
		fr.ipp = nclient.ipp;

		role = (to + HASHSIZE * 2 + 4 == 0x00) ? TCP_UBINDER 
											   : TCP_BINDER2;
		return this->newtunnel(sddr, fr, role, msg);
	}
	/**
	*	The client was not found - we create a request for
	*	its search in the network.
	*/
	storage::tasks.add(res, sddr, hash);
	return this->find_req(to);
}
/*********************************************************/
struct ret tunnels_router::newtunnel(struct sockaddr_in sddr,
									 struct route froute,
									 enum tcp_role role,
									 pack msg) {
	using storage::tunnels;
	using storage::tasks;

	unsigned char *target = msg.info(), *hash;
	unsigned char *from = target + HASHSIZE;
	size_t port, step = HASHSIZE * 2;
	pair<size_t, size_t> ports;
	pack msg1, msg2;

	msg1.tmp(NODE_RES_TUNNEL, msg.cookie());
	msg1.set_info(msg.info(), step);
	/**
	*	Add a return route (This will help not to look
	*	for a client, for a return answer).
	*/
	if (role == TCP_BINDER2) {
		storage::routes.set(true, from, msg.hash(),
							ipport_get(sddr));
	}
	/**
	*	If this request is repeated and the tunnel has
	*	already been created before.
	*/
	if (tunnels.find_ports(target, from, ports)) {
		msg1.add_info(step, &ports.second, 4);

		return handler::make_ret(sddr, msg.hash(),
								 msg1);
	}

	hash = (role == TCP_BINDER1) ? froute.father
								 : froute.hash;
	sddr = sddr_get(froute.ipp);
	port = tunnels.free_port();
	/**
	*	If we are the last binder for sending a message,
	*	we create an additional one later for sending to
	*	the end user.
	*/
	if (role != TCP_BINDER1) {
		tunnels.add(target, from, {TCP_SND, target, role,
								  {port, froute.ipp.ip}});

		msg2.tmp(USER_REQ_TUNNEL);
		msg2.set_info(msg.info(), step);
		/**
		*	In the information to the end user, indicate
		*	the port on which you can receive the message.
		*/
		msg2.add_info(step, &port, 4);
		tasks.add(msg2, sddr, target);

		port = tunnels.free_port();
	}

	tunnels.add(target, from, {TCP_RCV, hash, role,
							  {port, ""}});
	msg1.add_info(step, &port, 4);

	tasks.add(msg1, sddr, msg.hash());

	msg2.tmp(NODE_REQ_TUNNEL);
	msg2.set_info(msg.info(), HASHSIZE * 2);
	/**
	*	Set the flag that the next node is the second
	*	binder.
	*/
	*target = (role == TCP_BINDER1) ? 0x01 : 0x00;
	msg2.add_info(step + 4, target, 1);

	return handler::make_ret(sddr, hash, msg2);
}
/*********************************************************/
struct ret tunnels_router::get_message(struct sockaddr_in sddr,
									   unsigned char *info,
									   unsigned char *hash,
									   size_t cookie) {
	using storage::tunnels;
	using structs::role;

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

	tunnels.add(info, info + HASHSIZE, {TCP_RCV, hash,
				TCP_TARGET, {port, ""}});

	res.tmp((role == UDP_NODE) ? NODE_RES_TUNNEL
							   : USER_RES_TUNNEL, cookie);
	return handler::make_ret(sddr, hash, res);
}
/*********************************************************/
void tunnels_router::res(struct sockaddr_in sddr,
						 pack msg) {
	using storage::tunnels;

	unsigned char *target = msg.info(), *from;
	unsigned char *hash = msg.hash();
	enum tcp_role role;
	struct route one;
	struct ret task;
	size_t port;
	pack req;

	storage::tasks.rm_cookie(msg.cookie());

	if (is_null(target + HASHSIZE * 2, 4)) {
		/**
		*	User has not been found yet, wait.
		*/
		return;
	}

	from = target + HASHSIZE;
	role = (*(from + HASHSIZE + 4) == 0x01) ? TCP_BINDER2
											: TCP_BINDER1;
	memcpy(&port, from + HASHSIZE, 4);
	/**
	*	If error with routing in the chain.
	*/
	if (*(from + HASHSIZE + 5) == 0x01) {
		task = this->find_req(target);
		storage::tasks.add(task.msg, task.sddr, task.hash);
		return;
	}

	if (!storage::tunnels.is_freeport(port)
		|| role == TCP_BINDER2) {
		return;
	}

	if (storage::routes.find(target, one)) {
		tunnels.add(target, from, {TCP_SND, hash, role,
								  {port, one.ipp.ip}});
		return;
	}

	one.ipp = ipport_get(sddr);
	one.ipp.port = port;
	/**
	*	We are the initiator of the request - we
	*	create a stream to send a message.
	*/
	tunnels.add(target, from, {TCP_SND, hash, TCP_SENDER,
							  one.ipp});
}



struct ret tunnels_router::find_req(unsigned char *to) {
	using storage::father;

	struct client node = structs::father.info;
	pack msg;
	int cmp;

	msg.tmp(NODE_REQ_FIND);
	msg.set_info(to, HASHSIZE);

	storage::routes.rm_hash(to);
	storage::routes.set(false, to, nullptr, {});

	storage::clients.mute.lock();

	for (auto &p : structs::clients) {
		cmp = father.cmp(to, p.hash, node.hash);

		if (p.is_node && cmp == 1) {
			node = p;
		}
	}

	storage::clients.mute.lock();

	return handler::make_ret(sddr_get(node.ipp),
							 node.hash, msg);
}