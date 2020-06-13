
#ifndef _SPRUCE_NETWORK_
#define _SPRUCE_NETWORK_

#include "spruce.hpp"
#include "storage.hpp"
#include "tcptunnel.hpp"
#include "router.hpp"

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

	public:
		atomic<bool> work = false;

		void start(void);
		udp_network(void)  {
			set_sockaddr(recv.sddr, UDP_PORT);
		}
		~udp_network(void) {
			if (!work) { return; }
			CLOSE_SOCKET(sock);

			thr1.join();
			thr2.join();
		}
};

class handler_queue {
	private:
		map<size_t, unsigned char *> queue;
		mutex mute;

		void add(size_t cookie, unsigned char *hash) {
			unsigned char *nhash = new unsigned char[HASHSIZE];
			assert(nhash);

			HASHCPY(nhash, hash);

			queue.insert(make_pair(cookie, nhash));
		}

	public:
		bool lock(pack &msg) {
			map<size_t, unsigned char *>::iterator it;
			int cmp;

			mute.lock();

			it = queue.find(msg.cookie());

			if (it == queue.end()) {
				this->add(msg.cookie(), msg.hash());
				mute.unlock();
				return false;
			}

			assert(it->second);
			cmp = memcmp(it->second, msg.hash(), HASHSIZE);

			if (cmp != 0) {
				this->add(msg.cookie(), msg.hash());
				mute.unlock();
				return false;
			}

			mute.unlock();

			return true;
		}

		void unlock(pack &msg) {
			map<size_t, unsigned char *>::iterator it;
			int cmp;

			mute.lock();
			it = queue.find(msg.cookie());

			if (it == queue.end()) {
				mute.unlock();
				return;
			}

			assert(it->second);
			cmp = memcmp(it->second, msg.hash(), HASHSIZE);

			if (cmp != 0) {
				mute.unlock();
				return;
			}

			delete[] it->second;
			queue.erase(it);

			mute.unlock();
		}
};

inline udp_network network;

namespace handler {
	inline handler_queue queue;

	struct ret make_ret(struct sockaddr_in,
						unsigned char *,
						pack);

	void node(pack, struct sockaddr_in);
	void user(pack, struct sockaddr_in);
	void role(pack, struct sockaddr_in);
}

namespace router {
	inline tunnels_router	tunnels;
	inline father_router	father;
	inline nodes_router		nodes;
	inline find_router		find;
}

#endif