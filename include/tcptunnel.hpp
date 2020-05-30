
#ifndef _SPRUCE_TCP_TUNNEL_
#define _SPRUCE_TCP_TUNNEL_

#include "spruce.hpp"

struct received_data {
	ofstream fd;
	string name;
};

enum tunnel_type { TCP_SND, TCP_RCV };

struct init_data {
	enum tunnel_type type;
	unsigned char *hash;
	enum tcp_role role;
	struct ipport ipp;
};

class tcp_tunnel {
	private:
		unsigned char ehash[HASHSIZE];
		struct sddr_structs srv, cln;
		struct received_data recvd;
		struct tcp_message msg;
		struct tunnel *it;
		ifstream sfd; // if data for sending in the file.
		thread thr;
		int sock;

		void recv_processing(unsigned char *, int, size_t);
		void set_sockaddr(struct sockaddr_in &, size_t);
		void sender(enum tcp_role, struct ipport);
		void send_processing(enum tcp_status);
		void receiver(enum tcp_role, size_t);
		int tcp_socket(size_t);
		void get_content(void);
		void thr_send(void);

	public:
		enum tcp_role role = TCP_SENDER;
		size_t r_port = 0, s_port = 0;
		atomic<bool> work = true;

		void init(struct init_data, struct tunnel *);
		~tcp_tunnel(void);
};


#endif
