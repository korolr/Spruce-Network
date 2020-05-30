
#ifndef _SPRUCE_NETWORK_
#define _SPRUCE_NETWORK_

#include "spruce.hpp"
#include "storage.hpp"
#include "tcptunnel.hpp"

struct ret {
	unsigned char hash[HASHSIZE];
	struct sockaddr_in sddr;
	bool empty;
	pack msg;
};

class udp_network {
	private:
		struct sddr_structs recv;
		thread thr1, thr2;
		int sock;

		void processing(struct sockaddr_in, pack);
		void send_package(struct udp_task);
		void send_thread(void);
		void recv_thread(void);
		int udp_socket(size_t);

	public:
		atomic<bool> work = false;

		void start(void);

		~udp_network(void) {
			if (!work) { return; }
			socket_close(sock);

			thr1.join();
			thr2.join();
		}
};

class father_router {
	private:
		struct ret from_user(struct sockaddr_in, pack);
		struct ret chain(struct sockaddr_in, pack);
		struct ret iamfather(unsigned char *);

	public:
		struct ret req(struct sockaddr_in, pack);
		void res(struct sockaddr_in, pack);
};

class nodes_router {
	public:
		struct ret req(struct sockaddr_in, pack);
		void res(struct sockaddr_in, pack);
};

class tunnels_router {
	private:
		struct ret newtunnel(struct sockaddr_in, struct route,
							 enum tcp_role, pack);
		struct ret get_message(struct sockaddr_in, unsigned char *,
							   unsigned char *, size_t);

	public:
		struct ret req(struct sockaddr_in, pack);
		void res(struct sockaddr_in, pack);
};

class find_router {
	public:
		struct ret req(struct sockaddr_in, pack);
		void res(struct sockaddr_in, pack);
};

namespace handler {
	struct ret make_ret(struct sockaddr_in, unsigned char *, pack);
	void node(pack, struct sockaddr_in);
	void user(pack, struct sockaddr_in);
	void role(pack, struct sockaddr_in);
}

namespace router {
	inline tunnels_router tunnels;
	inline father_router father;
	inline nodes_router nodes;
	inline find_router find;
}

inline udp_network network;

#endif