#include "../../include/network.hpp"
/*********************************************************/
struct ret father_router::req(struct sockaddr_in sddr,
							  pack msg) {
	/**
	*	If the request came from a registered user to search
	*	for the optimal paternal node.
	*/
	if (is_null(msg.info(), HASHSIZE)) {
		return this->from_user(sddr, msg);
	}
	/**
	*	If the request is not from the target client.
	*/
	return this->chain(sddr, msg);
}
/*********************************************************/
void father_router::res(struct sockaddr_in sddr, pack msg) {
	using structs::father;
	using structs::keys;

	unsigned char *info = msg.info(), *ip;
	struct ipport ipp = ipport_get(sddr);
	struct client nfather;
	struct dadreq req;
	pack res;
	int cmp;

	storage::tasks.rm_cookie(msg.cookie());

	if (is_null(info, HASHSIZE)) {
		return;
	}
	/**
	*	Check if the current response is for a registered
	*	customer. If so, send it to the user.
	*/
	if (storage::dadreqs.find(req, info)) {
		assert(ip = ip2bin(ipp.ip));

		res.tmp(NODE_RES_FATHER);
		res.set_info(msg.hash(), HASHSIZE);
		res.add_info(HASHSIZE, ip, 4);

		storage::tasks.add(res, req.sddr, req.hash);
		storage::dadreqs.rm_hash(req.hash);

		delete[] ip;
		return;
	}
	/**
	*	If the found paternal node is a better (hash
	*	check) option than the current one, install it.
	*/
	cmp = storage::father.cmp(keys.pub, info,
							  father.info.hash);

	if (cmp != 1) {
		return;
	}

	nfather.ipp.ip = bin2ip(info + HASHSIZE);
	HASHCPY(nfather.hash, info);
	nfather.ipp.port = UDP_PORT;

	storage::father.set(nfather);
}
/*********************************************************/
struct ret father_router::from_user(struct sockaddr_in sddr,
									pack msg) {
	using structs::father;

	unsigned char *hash = msg.hash();
	struct client node;
	pack res;
	int cmp;
	/**
	*	If the client resubmitted the search request, we
	*	don’t respond.
	*/
	if (storage::dadreqs.exists(hash)) {
		res.tmp(NODE_RES_FATHER, msg.cookie());
		return handler::make_ret(sddr, hash, res);
	}

	storage::dadreqs.add(sddr, hash);
	storage::clients.mute.lock();
	node = father.info;
	/**
	*	Search for the closest node to the network user
	*	for further forwarding of the request to it.
	*/
	for (auto p : structs::clients) {
		if (!p.is_node) {
			continue;
		}

		cmp = storage::father.cmp(hash, p.hash, node.hash);

		if (cmp == 1) {
			node = p;
		}
	}

	storage::clients.mute.unlock();

	cmp = memcmp(node.hash, father.info.hash, HASHSIZE);
	res.tmp(NODE_REQ_FATHER);
	res.set_info(hash, HASHSIZE);
	/**
	*	Next, we send a request to both the nearest node
	*	and our father node so that there is no loopback
	*	of the request.
	*/
	if (cmp != 0) {
		storage::tasks.add(res, sddr_get(father.info.ipp),
		                   father.info.hash);
	}

	storage::tasks.add(res, sddr_get(node.ipp), node.hash);
	res.tmp(NODE_RES_FATHER, msg.cookie());

	return handler::make_ret(sddr, hash, msg);
}
/*********************************************************/
struct ret father_router::chain(struct sockaddr_in sddr,
								pack msg) {
	using structs::father;
	using structs::keys;

	struct ipport ipp = ipport_get(sddr);
	unsigned char *info = msg.info(), *ip;
	unsigned char *hash = msg.hash();
	struct client node;
	pack res;
	int cmp;
	/**
	*	Add the ip address of the node from which the
	*	initial request came. Further, an answer will
	*	be sent to this address.
	*/
	if (is_null(info + HASHSIZE, 4)) {
		assert(ip = ip2bin(ipp.ip));
		HASHCPY(info + HASHSIZE + 4, hash);
		memcpy(info + HASHSIZE, ip, 4);
		delete[] ip;
	}

	storage::clients.mute.lock();
	node = structs::father.info;
	/**
	*	Search for the closest node to the network user
	*	for further forwarding of the request to it.
	*/
	for (auto p : structs::clients) {
		if (!p.is_node) {
			continue;
		}

		cmp = storage::father.cmp(info, p.hash, node.hash);

		if (cmp == 1) {
			node = p;
		}
	}

	storage::clients.mute.unlock();
	cmp = storage::father.cmp(info, keys.pub, node.hash);

	res.tmp(NODE_RES_FATHER, msg.cookie());
	storage::tasks.add(res, sddr, hash);
	/**
	*	If we are the best paternal node for the client,
	*	we generate a message.
	*/
	if (cmp == 1) {
		return this->iamfather(info);
	}

	res.tmp(NODE_REQ_FATHER);
	res.set_info(info, HASHSIZE * 2 + 4);

	return handler::make_ret(sddr_get(node.ipp),
							 node.hash, res);
}
/*********************************************************/
struct ret father_router::iamfather(unsigned char *bytes) {
	unsigned char *ip = bytes + HASHSIZE;
	struct ipport ipp = { UDP_PORT, bin2ip(ip) };
	pack res;

	res.tmp(NODE_RES_FATHER);
	res.set_info(bytes, HASHSIZE * 2 + 4);

	return handler::make_ret(sddr_get(ipp), bytes +
							 HASHSIZE + 4, res);
}
