
#ifndef _SPRUCE_PACK_PORTS_
#define _SPRUCE_PACK_PORTS_

#include "../pack.hpp"

class req_port : public pack_tmp {
public:
	req_port(const unsigned char *b = nullptr) : pack_tmp(b) {
		type = (structs::role == UDP_NODE) ? NODE_REQ_PORT
										   : USER_REQ_PORT;
	}

	size_t port(size_t rport = 0) {
		if (rport != 0) {
			memcpy(buffer, &rport, 4);
			return rport;
		}

		size_t oport;
		memcpy(&oport, buffer, 4);

		return oport;
	}
};

class res_port : public req_port {
public:
	res_port(const unsigned char *b = nullptr) : req_port(b) {
		type = (structs::role == UDP_NODE) ? NODE_RES_PORT
										   : USER_RES_PORT;
	}

	bool accepted(bool flag = false, bool set = false) {
		unsigned char code = (flag) ? 0x01 : 0x00;

		if (set) {
			memset(buffer + 4, code, 1);
		}

		return *(buffer + 4) == 0x01;
	}
};

#endif