
#include "../include/network.hpp"

void udp_network::start(void) {
	sock = new_socket(SOCK_DGRAM, TIMEOUT);
	work = true;

	assert(sock != -1);

	thr1 = thread(&udp_network::send_thread, this);
	thr2 = thread(&udp_network::recv_thread, this);

	if (!thr1.joinable() || !thr2.joinable()) {
		work = false;
	}
}
/*********************************************************/
void udp_network::handling(struct sockaddr_in sddr,
						   pack msg) {
	THREAD_START();

	{
		using storage::clients;
		using structs::role;

		bool is_ping = msg.type() == USER_REQ_PING
					|| msg.type() == NODE_REQ_PING;

		storage::nodes.sub_attempts(msg.hash());

		switch (role) {
		case UDP_NODE:
			clients.reg(msg.hash(), ipport_get(sddr),
						msg.role(), is_ping);

			handler::node(msg, sddr);
			break;

		case UDP_USER:
			handler::user(msg, sddr);
			break;

		default:
			if (msg.type() == RES_FATHER) {
				router::father.response(sddr, msg);
				break;
			}

			if (msg.type() == DOS) {
				//storage::nodes.rm_hash(msg.hash());
				break;
			}

			if (msg.type() != RES_ROLE) {
				break;
			}

			role = (*msg.info() == UDP_NODE) ? UDP_NODE
											 : UDP_USER;
		}
	}

	THREAD_END();
}
/*********************************************************/
void udp_network::send_thread(void) {
	using structs::father;
	using storage::tasks;
	using structs::role;

	time_point<system_clock> time, ping;
	struct sockaddr_in sddr;
	struct udp_task task;
	bool sendt;
	pack msg;

	ping = system_clock::now();

	while ((time = system_clock::now()), work) {
		/**
		*	Checking the paternal node.
		*/
		if (time - father.info.ping > 15s && father.status) {
			storage::father.no_father();
			continue;
		}
		/**
		*	If we just connected to the network and we have
		*	not yet been assigned a role.
		*/
		if (role == UDP_NONE && !tasks.exists(REQ_ROLE)
			&& father.status) {
			msg.tmp(REQ_ROLE);
			tasks.add(msg, sddr_get(father.info.ipp),
					  father.info.hash);
		}

		sendt = tasks.first(task);
		/**
		*	We receive a package for sending if we have
		*	already received a role in the network.
		*
		*	If there are no messages to send, create a
		*	PING packet to the father node.
		*/
		if (!sendt && role != UDP_NONE && father.status
			&& system_clock::now() - ping >= 2s) {
			msg.tmp((role == UDP_USER) ? USER_REQ_PING
									   : NODE_REQ_PING);
			sddr = sddr_get(father.info.ipp);
			task = msg.to_task(father.info.hash, &sddr);
			ping = system_clock::now();
			sendt = true;
		}

		if (sendt) {
			send_package(task);
		}
	}
}
/*********************************************************/
void udp_network::send_package(struct udp_task task) {
	struct sockaddr *ptr;
	socklen_t size;

	ptr = reinterpret_cast<struct sockaddr *>(&task.sddr);
	size = sizeof(struct sockaddr_in);
	task.sddr.sin_family = AF_INET;

	int s = sendto(sock, task.buff, task.len, 0, ptr, size);
}
/*********************************************************/
void udp_network::recv_thread(void) {
	socklen_t sz = sizeof(struct sockaddr_in);
	unsigned char buff[MAX_UDPSZ];
	size_t rs, psz = MAX_UDPSZ;
	struct sddr_structs cln;
	pack msg;

	assert(bind(sock, recv.ptr, sz) == 0);

	while (work) {
		/**
		*	We receive a packet from a network participant,
		*	check it and create a stream for processing.
		*/
		rs = recvfrom(sock, buff, psz, MSG_DONTWAIT, cln.ptr, &sz);
		msg = pack(buff, rs);

		if (rs < UDP_PACK || !msg.is_correct()) {
			continue;
		}
		/**
		*	A new thread is created so as not to delay the
		*	message receiving cycle.
		*/
		thread(&udp_network::handling, this, cln.sddr, msg).detach();

		memset(buff, 0x00, psz);
	}
}

