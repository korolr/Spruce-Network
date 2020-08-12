
#ifndef _SPRUCE_ROUTER_
#define _SPRUCE_ROUTER_

#include "network.hpp"

class father_router {
	private:
		struct udp_father dad;
		size_t attempts = 0;
		mutex mute;

	public:
		struct ret
		request(struct sockaddr_in, pack);

		void
		response(struct sockaddr_in, pack);
};

class find_router {
	struct freqs  {
		unsigned char hash[HASHSIZE];
		time_point<system_clock> time;
		string ip;
	};

	private:
		vector<struct freqs> reqs;
		mutex mute;

		bool
		allow(struct sockaddr_in, unsigned char *);

		void 
		find_processing(struct sockaddr_in, pack);

		void
		packet_distribution(req_find);

	public:
		struct ret
		request(struct sockaddr_in, pack);
};

class port_router {
	public:
		struct ret
		request(struct sockaddr_in, pack);

		void
		response(struct sockaddr_in, pack);
};

class message_router {
	private:
		void
		message_processing(req_msg &);

	public:
		struct ret
		request(struct sockaddr_in, pack);
};

#endif