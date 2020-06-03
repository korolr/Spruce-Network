
#ifndef _SPRUCE_PACK_
#define _SPRUCE_PACK_

#include "spruce.hpp"
#include "encryption.hpp"

class pack {
	private:
		unsigned char left[100], right[100];
		unsigned char buffer[UDP_PACK];
		size_t trash_size;
		bool correct;

		void gentrash_nix(void);
		void gentrash_win(void);

	public:
		struct udp_task to_task(unsigned char *, struct sockaddr_in *sddr = nullptr);
		void tmp(enum udp_type type = INDEFINITE, size_t cookie = 0);
		pack(unsigned char *buff = nullptr, size_t size = 0);
		void add_info(size_t,  void *, size_t);
		void set_info(unsigned char *, size_t);
		unsigned char *bytes(void);
		unsigned char *hash(void);
		unsigned char *info(void);
		enum udp_type type(void);
		enum udp_role role(void);
		void set_correct(void);
		string hash_str(void);
		bool is_correct(void);
		size_t cookie(void);
		size_t is_req(void);
#if defined(DEBUG) && DEBUG == true
		void debug(void);
#endif
};

#endif
