
#include "../../include/network.hpp"
/*********************************************************/
struct ret nodes_router::req(struct sockaddr_in sddr,
							 pack msg) {
	size_t max = (UDP_PACK - HASHSIZE - 5) / (HASHSIZE + 4);
	size_t tmp, size = max * (HASHSIZE + 4);
	unsigned char buff[size], *ip;
	pack res;

	memset(buff, 0x00, size);
	storage::clients.mute.lock();
	/**
	*	Generation of a byte array in the format: host
	*	hash, host ip address.
	*/
	for (auto p : structs::clients) {
		if (!p.is_node) {
			continue;
		}

		tmp = (max - 1) * (HASHSIZE + 4);
		assert(ip = ip2bin(p.ipp.ip));

		memcpy(buff + tmp + HASHSIZE, ip, 4);
		HASHCPY(buff + tmp, p.hash);

		delete[] ip;

		if (max == 0) {
			break;
		}

		max--;
	}

	storage::clients.mute.unlock();

	res.tmp(NODE_RES_NODE, msg.cookie());
	res.set_info(buff, size);

	return handler::make_ret(sddr, msg.hash(), res);
}
/*********************************************************/
void nodes_router::res(struct sockaddr_in sddr, pack msg) {
	size_t max = (UDP_PACK - HASHSIZE - 5) / (HASHSIZE + 4);
	unsigned char hash[HASHSIZE], *info = msg.info();
	size_t tmp;
	string ip;

	storage::tasks.rm_cookie(msg.cookie());

	for (size_t i = 0; i < max; i++) {
		tmp = (max - 1) * (HASHSIZE + 4);

		ip = bin2ip(info + tmp + HASHSIZE);
		HASHCPY(hash, info + tmp);

		if (is_null(hash, HASHSIZE) || ip.length() < 5) {
			max--;
			continue;
		}

		storage::nodes.add(hash, ip);
	}
}