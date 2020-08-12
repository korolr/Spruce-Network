
#ifndef _SPRUCE_PACK_TUNNEL_
#define _SPRUCE_PACK_TUNNEL_

#include "../pack.hpp"
#define TUNNSIZE HASHSIZE + 8
//[ROLE IN TUNNEL][KEY][ROUTE HASH][ROUTE IP]
class req_tunnel : public pack_tmp {
public:
	req_tunnel(const unsigned char *b = nullptr) : pack_tmp(b) {
		type = (structs::role == UDP_NODE) ? NODE_REQ_TUNNEL
										   : USER_REQ_TUNNEL;
	}

	enum tcp_role role(enum tcp_role r = TCP_NONE) {
		if (r != TCP_NONE) {
			*buffer = static_cast<unsigned char>(r);
		}

		return static_cast<enum tcp_role>(*buffer);
	}

	size_t code(size_t c = 0) {
		if (c != 0) {
			memcpy(buffer + 1, &c, 4);
			return c;
		}

		size_t code = 0;

		memcpy(&code, buffer + 1, 4);
		return code;
	}

	unsigned char *hash(unsigned char *b = nullptr) {
		if (b != nullptr) {
			HASHCPY(buffer + 5, b);
			return b;
		}

		return buffer + 5;
	}

	string ipaddr(string ip = "") {
		if (ip.length() == 0) {
			return bin2ip(buffer + 5 + HASHSIZE);
		}

		unsigned char *b = ip2bin(ip);
		assert(b);

		memcpy(buffer + 5 + HASHSIZE, b, 4);
		delete[] b;
		return ip;
	}
};

#endif