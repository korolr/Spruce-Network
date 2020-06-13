
#ifndef _SPRUCE_ROUTER_
#define _SPRUCE_ROUTER_

#include "network.hpp"

class father_router {
	private:
		struct ret
		from_user(struct sockaddr_in,
				  pack);

		struct ret
		chain(struct sockaddr_in, pack);

		struct ret
		iamfather(unsigned char *);

	public:
		struct ret
		req(struct sockaddr_in, pack);

		void
		res(struct sockaddr_in, pack);
};

class nodes_router {
	public:
		struct ret
		req(struct sockaddr_in, pack);

		void
		res(struct sockaddr_in, pack);
};

class tunnels_router {
	private:
		struct ret
		create_tunnel(enum tcp_role,
					  struct sockaddr_in,
					  struct route,
					  pack);

		struct ret
		get_message(struct sockaddr_in,
					unsigned char *,
					unsigned char *,
					size_t);

	public:
		struct ret
		req(struct sockaddr_in, pack);

		void
		res(struct sockaddr_in, pack);
};

class find_router {

	struct find_req {
		unsigned char hash[HASHSIZE],
					  from[HASHSIZE];
		time_point<system_clock> time;
	};

	private:
		vector<struct find_req> reqs;
		mutex mute;

		struct ret
		from_client(struct sockaddr_in,
					pack);

		struct ret
		from_chain(struct sockaddr_in,
				   pack);

		bool
		allow_request(pack);

	public:
		struct ret
		req(struct sockaddr_in, pack);

		void
		res(struct sockaddr_in, pack);
};

#endif