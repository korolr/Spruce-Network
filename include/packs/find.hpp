
#ifndef _SPRUCE_PACK_FIND_
#define _SPRUCE_PACK_FIND_

#include "../pack.hpp"
#define FINDVERIF HASHSIZE * 2 + 8
// [HASH WHO][ECNRYPTED [FROM][CODE][HASH DAD][IP DAD]]
class req_find : public pack_tmp {
private:
	unsigned char ebuffer[FINDVERIF];

	void write_encrypted(unsigned char *key) {
		constexpr size_t size = FINDVERIF + crypto_box_SEALBYTES;
		assert(key);

		unsigned char *e = encryption::pack(key, ebuffer, FINDVERIF);
		memcpy(buffer + HASHSIZE, e, size);
		delete[] e;
	}

public:
	bool decode = false;

	req_find(const unsigned char *b = nullptr) : pack_tmp(b) {
		type = (structs::role == UDP_NODE) ? NODE_REQ_FIND
										   : USER_REQ_FIND;

		unsigned char *e = encryption::unpack(buffer + HASHSIZE,
											  FINDVERIF);
		if (e) {
			memcpy(ebuffer, e, FINDVERIF);
			decode = true;
			delete[] e;
		}
	}

	unsigned char *who(unsigned char *h = nullptr) {
		if (h != nullptr) {
			HASHCPY(buffer, h);
		}

		return buffer;
	}

	unsigned char *from(unsigned char *b = nullptr,
						unsigned char *h = nullptr) {
		if (b != nullptr && h != nullptr) {
			HASHCPY(ebuffer, b);
			write_encrypted(h);
		}

		return ebuffer;
	}

	size_t code(size_t c = 0, unsigned char *h = nullptr) {
		if (c != 0 || h != nullptr) {
			memcpy(ebuffer + HASHSIZE, &c, 4);
			write_encrypted(h);
			return c;
		}

		size_t num = 0;
		memcpy(&num, ebuffer + HASHSIZE, 4);
		return num;
	}

	struct haship father(struct haship data = {},
						 unsigned char *h = nullptr) {
		if (data.ip.length() == 0 || h == nullptr) {
			struct haship st;

			st.ip = bin2ip(ebuffer + HASHSIZE * 2 + 4);
			HASHCPY(st.hash, ebuffer + HASHSIZE + 4);

			return st;
		}

		unsigned char *ip = ip2bin(data.ip);
		assert(ip);

		HASHCPY(ebuffer + HASHSIZE + 4, data.hash);
		memcpy(ebuffer  + HASHSIZE * 2 + 4, ip, 4);

		write_encrypted(h);
		delete[] ip;

		return data;
	}
};

#endif