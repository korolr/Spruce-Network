
#ifndef _SPRUCE_TCP_TUNNEL_
#define _SPRUCE_TCP_TUNNEL_

#include "spruce.hpp"

#define NORMAL_STATUS(c)					\
	((c) >= TUNNEL_1 && (c) <= ERROR_T)

enum tunnel_type { TCP_SND, TCP_RCV };

struct init_tunnel {
	enum tunnel_type type;
	unsigned char *hash;
	enum tcp_role role;
	struct ipport ipp;
};

class tcp_tunnel {
	private:
		unsigned char ehash[HASHSIZE];
		struct sddr_structs srv, cln;
		struct tunnel *it;
		thread thr;
		int sock;

		void recv_processing(unsigned char *, int, size_t);
		void send_processing(enum tcp_status, bool);
		void sender(enum tcp_role, struct ipport);
		void receiver(enum tcp_role, size_t);
		void thr_send(void);

	public:
		enum tcp_role role = TCP_SENDER;
		size_t r_port = 0, s_port = 0;
		atomic<bool> work = true;

		void init(struct init_tunnel, struct tunnel *);
		~tcp_tunnel(void);
};


#endif
