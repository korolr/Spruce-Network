
#ifndef _SPRUCE_TCPTUNNEL_
#define _SPRUCE_TCPTUNNEL_

#include "spruce.hpp"
#include "encryption.hpp"
#define TUNNPACK PACKLEN + crypto_box_SEALBYTES

class tcp_tunnel {
public:
	struct tcp_data {
		time_point<system_clock> time;
		unsigned char hash[HASHSIZE];
		struct sddr_structs own, srv;
		atomic<bool> start = false;
		int sock, usock;
		thread thr;
	}	send_init, recv_init;

	atomic<bool> work = false, error = true;
	enum tcp_role role = TCP_NONE;

	tcp_tunnel(enum tcp_role *prole,
			   struct client *pnode,
			   struct client *puser,
			   unsigned char *phash) {

		assert(prole && pnode && puser && phash);
		init = { pnode, puser, prole, phash };
	}

	~tcp_tunnel(void) {
		if (send_init.thr.joinable()) {
			send_init.thr.join();
		}

		if (recv_init.thr.joinable()) {
			recv_init.thr.join();
		}

		CLOSE_SOCKET(recv_init.sock);
		CLOSE_SOCKET(recv_init.usock);
		CLOSE_SOCKET(send_init.sock);
		CLOSE_SOCKET(send_init.usock);

		work = false;
	}

	void start(bool, size_t);

private:
	struct {
		struct client *node, *user;
		enum tcp_role *role;
		unsigned char *hash;
	}	init;

	unsigned char buffer[TUNNPACK];
	size_t length = 0;
	mutex mute;

	bool connect_chain(tcp_tunnel::tcp_data &, bool);
	void send_thread(bool);
	void recv_thread(bool);

	size_t buff_size(void) {
		mute.lock(); size_t size = length; mute.unlock();
		return size;
	}
};

struct tunnel {
	time_point<system_clock> time;
	unsigned char hash[HASHSIZE]; // Can be target hash or hash of msg owner. 
	struct client node, user;
	enum tcp_role role;
	size_t id, code;

	struct haship binder1;
	tcp_tunnel tcp;

	bool sync(void) {
		return (role >= TCP_TARGET) ? tcp.recv_init.start
									: tcp.send_init.start;
	}

	tunnel(void) : tcp(&role, &node, &user, hash) { }
};

#endif
