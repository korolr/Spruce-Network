
#ifndef _SPRUCE_PACK_
#define _SPRUCE_PACK_

#include "spruce.hpp"
#include "encryption.hpp"

#define VALID_TYPE(t) \
	((t) < USER_END || ((t) > USER_END && (t) < NODE_END) \
	|| (t) == REQ_ROLE || (t) == DOS || (t) == REQ_FATHER \
	|| (t) == RES_ROLE || (t) == RES_FATHER) 


class pack_tmp {
public:
	unsigned char buffer[INFOSIZE];
	enum udp_type type;
	size_t cookie = 0;

	pack_tmp(const unsigned char *b) {
		if (!b) {
			memset(buffer, 0x00, INFOSIZE);
			return;
		}

		memcpy(buffer, b, INFOSIZE);
	}

	void clear(void) {
		memset(buffer, 0x00, INFOSIZE);
		cookie = 0;
	}
};


class pack {
private:
	unsigned char left[100], right[100];
	unsigned char buffer[UDP_PACK];
	bool correct = false;
	size_t tsize = 0;

	void gentrash_nix(void);
	void gentrash_win(void);

public:
	template<typename T> void to(T &var) {
		assert(correct);

		var = T(buffer + HASHSIZE + 5);
		var.cookie = cookie();
	}

	template<typename T> void from(const T var) {
		using structs::role;

		memcpy(buffer + 5 + HASHSIZE, var.buffer, INFOSIZE);
		memset(buffer, var.type, 1);

		size_t cookie = (var.cookie == 0)
								  ? random_cookie()
								  : var.cookie;
		unsigned char *key = (role == UDP_NODE) 
								  ? structs::keys.pub
								  : structs::fkeys.pub;
		memcpy(buffer + HASHSIZE + 1, &cookie, 4);
		HASHCPY(buffer + 1, key);
		correct = true;
	}

	pack(unsigned char *b = nullptr,size_t s = 0);
	struct udp_task to_task(unsigned char *,
							struct sockaddr_in *);
	void set(const unsigned char *, size_t);
	void tmp(enum udp_type, size_t c = 0);
	unsigned char *info(void);
	unsigned char *hash(void);
	enum udp_type  type(void);
	void change_cookie(void);
	enum udp_role role(void);
	bool is_correct(void);
	string hash_str(void);
	size_t   cookie(void);
#if defined(DEBUG) && DEBUG == true
	string type_str(void);
	void debug(void);
#endif
};

#include "packs/tunnel.hpp"
#include "packs/father.hpp"
#include "packs/ports.hpp"
#include "packs/find.hpp"
#include "packs/msg.hpp"

#endif
