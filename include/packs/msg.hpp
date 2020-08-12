

#ifndef _SPRUCE_PACK_MSG_
#define _SPRUCE_PACK_MSG_

#include "../pack.hpp"
#define MSGSIZE HASHSIZE * 2 + 8 

//[ENCRYPTED [HASH FROM][CODE][NODE HASH][NODE IP]][RESEND HASH][RESEND IP]
class req_msg : public pack_tmp {
private:
	unsigned char ebuffer[INFOSIZE];
	
	void write_encrypted(unsigned char *key) {
		constexpr size_t size = MSGSIZE + crypto_box_SEALBYTES;
		unsigned char *e;
		assert(key);

		e = encryption::pack(key, ebuffer, MSGSIZE);
		memcpy(buffer, e, size);
		delete[] e;
	}

public:
	bool decode = false;

	req_msg(const unsigned char *b = nullptr) : pack_tmp(b) {
		type = (structs::role == UDP_NODE) ? NODE_REQ_MSG
										   : USER_REQ_MSG;

		unsigned char *e = encryption::unpack(buffer, MSGSIZE);

		if (e != nullptr) {
			memcpy(ebuffer, e, MSGSIZE);
			decode = true;
			delete[] e;
		}
	}

	unsigned char *from(unsigned char *b = nullptr,
						unsigned char *h = nullptr) {
		if (b != nullptr && h != nullptr) {
			HASHCPY(ebuffer, b);
			write_encrypted(h);
			return b;
		}

		return ebuffer;
	}

	size_t code(size_t k = 0, unsigned char *h = nullptr) {
		if (k != 0 && h != nullptr) {
			memcpy(ebuffer + HASHSIZE, &k, 4);
			write_encrypted(h);
			return k;
		}

		size_t key = 0;

		memcpy(&key, ebuffer + HASHSIZE, 4);
		return key;
	}

	unsigned char *hash(unsigned char *b = nullptr,
						unsigned char *h = nullptr) {
		if (b != nullptr && h != nullptr) {
			HASHCPY(ebuffer + HASHSIZE + 4, b);
			write_encrypted(h);
			return b;
		}

		return ebuffer + HASHSIZE + 4;
	}

	string ipaddr(string ip = "", unsigned char *h = nullptr) {
		if (ip.length() == 0 || h == nullptr) {
			return bin2ip(ebuffer + HASHSIZE * 2 + 4);
		}

		unsigned char *b = ip2bin(ip);
		assert(b);

		memcpy(ebuffer + HASHSIZE * 2 + 4, b, 4);
		write_encrypted(h);
		delete[] b;
		return ip;
	}

	struct haship resend(struct haship hi = {},
						 bool set_null = false) {
		constexpr size_t size = crypto_box_SEALBYTES + MSGSIZE;

		if (hi.ip.length() != 0 && !IS_NULL(hi.hash, HASHSIZE)) {
			unsigned char *ip = ip2bin(hi.ip);
			assert(ip);

			memcpy(buffer + size + HASHSIZE, ip, 4);
			HASHCPY(buffer + size, hi.hash);
			delete[] ip;

			return hi;
		}

		struct haship out;

		if (set_null) {
			memset(buffer + size, 0x00, HASHSIZE + 4);
			return out;
		}

		out.ip = bin2ip(buffer + size + HASHSIZE);
		HASHCPY(out.hash, buffer + size);

		return out;
	}
};

#endif