
#ifndef _SPRUCE_STORAGE_
#define _SPRUCE_STORAGE_

#include "spruce.hpp"

class tasks_handler {
	private:
		mutex mute;

	public:
		void add(pack, struct sockaddr_in, unsigned char *);
		void rm_hash(unsigned char *, size_t);
		bool first(struct udp_task &);
		bool exists(enum udp_type);
		void rm_cookie(size_t);
		void renew(void);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class nodes_handler {
	private:
		time_point<system_clock> ping;
		mutex mute;

		void from_clients(void);

	public:
		nodes_handler(void) { ping = system_clock::now() - 600s; }
		void insert(unsigned char *, string);
		void add(unsigned char *, string);
		void rm_hash(unsigned char *);
		void temporary_father(void);
		void select(void);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class clients_handler {
	public:
		mutex mute;

		void reg(unsigned char *, struct ipport, enum udp_role, bool);
		bool find(unsigned char *, struct client &);
		struct client nearest(unsigned char *);
		void rm_hash(unsigned char *);
		bool exists(unsigned char *);
		void rm_ip(string);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class father_handler {
	private:
		time_point<system_clock> ping;
		mutex mute;

	public:
		size_t cmp(unsigned char *, unsigned char *, unsigned char *);
		father_handler(void) { ping = system_clock::now() - 600s; }
		bool from_father(struct sockaddr_in, pack);
		void set(struct client);
		void no_father(void);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class dadreqs_handler {
	private:
		mutex mute;

	public:
		void add(struct sockaddr_in, unsigned char *);
		bool find(struct dadreq &, unsigned char *);
		void rm_hash(unsigned char *);
		bool exists(unsigned char *);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class db_handler {
	private:
		sqlite3 *db;
		mutex mute;

		void create_tables(void);

	public:
		bool get_father(unsigned char *, string &);
		map<unsigned char *, string> nodes(void);
		db_handler(string path = "./spruce.db");
		void add_node(unsigned char *, string);
		void set_father(struct udp_father);
		void set_var(string, string);
		string get_var(string);
		void rm_node(string);
		void rm_var(string);
		~db_handler(void);
};

class routes_handler {
	private:
		mutex mute;

	public:
		void update(unsigned char *, unsigned char *, struct ipport);
		void set(bool, unsigned char *, unsigned char *, struct ipport);
		bool find(unsigned char *, vector<struct route>::iterator &);
		void rm_hash(unsigned char *);
		bool exists(unsigned char *);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class tunnels_handler {
	private:
		struct sddr_structs st;
		mutex mute;

	public:
		bool find(unsigned char *, unsigned char *, enum tcp_role, struct tunnel **);
		bool find_ports(unsigned char *, unsigned char *, pair<size_t, size_t> &);
		void add(unsigned char *, unsigned char *, struct init_tunnel);
		tunnels_handler(void) { set_sockaddr(st.sddr); }
		bool sys_freeport(size_t);
		bool is_freeport(size_t);
		size_t free_port(void);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class finds_handler {
	private:
		mutex mute;

	public:
		void update(unsigned char *, unsigned char);
		size_t check(unsigned char *);
		void add(unsigned char *);
};


class msgs_handler {
	private:
		map<unsigned char *, size_t> threads;
		mutex mute;

		void user_thread(size_t, struct tunnel *);
		size_t thread_exists(unsigned char *);
		void remove_thread(unsigned char *);

	public:
		size_t create_thread(unsigned char *);
		void add(unsigned char *);
		void check(void);
};

class inbox_handler {
	public:
		mutex mute;

		void add(unsigned char *hash, size_t port) {
			struct hashport one;

			assert(hash && port > 0);

			HASHCPY(one.hash, hash);
			one.port = port;

			mute.lock();
			structs::api::inbox.push_back(one);
			mute.unlock();
		}
};

namespace storage {
	inline dadreqs_handler		dadreqs;
	inline clients_handler		clients;
	inline tunnels_handler		tunnels;
	inline father_handler		father;
	inline routes_handler		routes;
	inline tasks_handler		tasks;
	inline nodes_handler		nodes;
	inline inbox_handler		inbox;
	inline finds_handler		finds;
	inline msgs_handler			msgs;
	inline db_handler			db;

	inline void check(void) {
		dadreqs.check(); father.check(); routes.check();
		tunnels.check(); clients.check(); nodes.check();
		msgs.check(); tasks.check();
	}
}

#endif