// hash
// hash - ip req node - hash
//
#include "../../include/network.hpp"
/*********************************************************/
struct ret find_router::req(struct sockaddr_in sddr,
							pack msg) {
	using structs::father;
	using structs::keys;
	using storage::tasks;

	unsigned char *info = msg.info(), *tmp;
	bool found = false, first = false;
	struct client fclient;
	struct ipport ipp;
	pack msg1, msg2;
	size_t cmp;
	bool res;

	msg1.tmp(NODE_RES_FIND, msg.cookie());

	if (is_null(info, HASHSIZE)) {
		return handler::make_ret(sddr, msg.hash(), msg1);
	}
	/**
	*	Adding host information that initiates a client
	*	search.
	*/
	if (is_null(info + HASHSIZE, 4)) {
		assert(tmp = ip2bin((ipport_get(sddr)).ip));
		msg.add_info(HASHSIZE + 4, msg.hash(), HASHSIZE);
		msg.add_info(HASHSIZE, tmp, 4);
		first = true;
		delete[] tmp;
	}

	storage::clients.mute.lock();
	fclient = structs::clients[0];
	/**
	*	Searching for the right client among registered
	*	users or the nearest node to the desired network
	*	client with us.
	*/
	for (auto &p : structs::clients) {
		cmp = storage::father.cmp(info, p.hash,
		                          fclient.hash);

		if (p.is_node && cmp == 1) {
			fclient = p;
		}

		if (cmp != 0) {
			continue;
		}

		found = true;
		break;
	}

	storage::clients.mute.unlock();
	/**
	*	If the desired client was found at the second node
	*	in the chain (if there is no information to send a
	*	response - a sign that the second node is in the
	*	chain).
	*/
	if (first && found) {
		msg1.set_info(info, HASHSIZE);
		return handler::make_ret(sddr, msg.hash(), msg1);
	}

	if (found) {
		ipp = { UDP_PORT, bin2ip(info + HASHSIZE) };
		msg2.tmp(NODE_RES_FIND);
		msg2.set_info(info, HASHSIZE);

		tasks.add(msg2, sddr_get(ipp), info + HASHSIZE +
				  4);
		return handler::make_ret(sddr, msg.hash(), msg1);
	}
	/**
	*	If the user you are looking for is the father or
	*	if the father is the more suitable site for the
	*	search, or if we are the desired customer.
	*/
	cmp = storage::father.cmp(info, father.info.hash,
							  fclient.hash);
	msg2.tmp(NODE_REQ_FIND);
	msg2.set_info(info, HASHSIZE * 2 + 4);
	tmp = father.info.hash;

	res = memcmp(info, tmp, HASHSIZE) == 0 || cmp == 1
		|| IS_ME(info);

	if (res) {
		ipp = father.info.ipp;
		tasks.add(msg2, sddr_get(ipp), father.info.hash);
		return handler::make_ret(sddr, msg.hash(), msg1);
	}
	/**
	*	If all the previous conditions have not passed,
	*	we send a search request to the node that was
	*	found in the loop
	*/
	tasks.add(msg2, sddr_get(fclient.ipp), fclient.hash);
	return handler::make_ret(sddr, msg.hash(), msg1);
}
/*********************************************************/
void find_router::res(struct sockaddr_in sddr, pack msg) {
	unsigned char *info = msg.info();
	struct ipport ipp;
	struct route fr;
	pack res;

	storage::tasks.rm_cookie(msg.cookie());

	if (is_null(info, HASHSIZE)
		|| !storage::routes.find(info, fr)) {
		return;
	}

	ipp = { UDP_PORT, bin2ip(info += HASHSIZE) };
	storage::routes.update(info, info + 4, ipp);
}