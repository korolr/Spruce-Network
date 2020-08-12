
#ifndef _SPRUCE_NETWORK_
#define _SPRUCE_NETWORK_

#include "spruce.hpp"
#include "storage.hpp"
#include "router.hpp"
#include "pack.hpp"

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

		void handling(struct sockaddr_in, pack);
		void send_package(struct udp_task);
		void send_thread(void);
		void recv_thread(void);

	public:
		atomic<bool> work = false;

		void start(void);
		udp_network(void)  {
			set_sockaddr(recv.sddr, UDP_PORT);
		}
		~udp_network(void) {
			work = false;
			CLOSE_SOCKET(sock);

			if (thr1.joinable()) {
				thr1.join();
			}
			
			if (thr2.joinable()) {
				thr2.join();
			}
		}
};

class handler_queue {
private:
	map<size_t, string> queue;
	mutex mute;

public:
	bool lock(pack &msg, string &ip) {
		mute.lock();
		auto it = queue.find(msg.cookie());

		if (it != queue.end() && it->second == ip) {
			mute.unlock();
			return true;
		}

		queue.insert(make_pair(msg.cookie(), ip));
		mute.unlock();
		
		return false;
	}

	void unlock(pack &msg, string &ip) {
		mute.lock();
		auto it = queue.find(msg.cookie());

		if (it != queue.end() && it->second == ip) {
			queue.erase(it);
		}

		mute.unlock();
	}
};

inline udp_network network;

namespace handler {
	inline handler_queue queue;

	struct ret make_ret(struct sockaddr_in,
						unsigned char *, pack);

	void father(pack, struct sockaddr_in);
	void node(pack,   struct sockaddr_in);
	void user(pack,   struct sockaddr_in);
}

namespace router {
	//inline tunnels_router	tunnels;
	inline message_router	message;
	inline father_router	father;
	inline port_router		port;
	inline find_router		find;
	/*inline nodes_router		nodes;
	inline verif_router		verif;*/
}

#endif