
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

		void reg(unsigned char *, struct ipport, enum udp_role);
		bool find(unsigned char *, struct client &);
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

class database_handler {
	private:
		sqlite3 *db;
		mutex mute;

		void create_tables(void);

	public:
		database_handler(string path = "./spruce.db");
		bool get_father(unsigned char *, string &);
		map<unsigned char *, string> nodes(void);
		void add_node(unsigned char *, string);
		void set_father(struct udp_father);
		void set_var(string, string);
		~database_handler(void);
		string get_var(string);
		void rm_node(string);
		void rm_var(string);
};

class routes_handler {
	private:
		mutex mute;

	public:
		void update(unsigned char *, unsigned char *, struct ipport);
		void set(bool, unsigned char *, unsigned char *, struct ipport);
		bool find(unsigned char *, struct route &);
		void rm_hash(unsigned char *);
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
		bool find_ports(unsigned char *, unsigned char *, pair<size_t, size_t> &);
		void add(unsigned char *, unsigned char *, struct init_data);
		tunnels_handler(void) { set_sockaddr(st.sddr); }
		bool sys_freeport(size_t);
		bool is_freeport(size_t);
		size_t free_port(void);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class messages_handler {
	private:
		mutex mute;

		size_t free_id(void);

	public:
		size_t add_bytes(unsigned char *, unsigned char *, size_t);
		bool find_hash(unsigned char *, struct tcp_message &);
		size_t add_file(unsigned char *, string);
		size_t info(size_t);
		void rm_id(size_t);
		void done(size_t);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class inbox_handler {
	private:
		mutex mute;

		size_t free_id(void);

	public:
		void add(unsigned char *, string);
		map<string, string> list(void);
		void rm_id(size_t);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

namespace storage {
	inline dadreqs_handler dadreqs;
	inline clients_handler clients;
	inline tunnels_handler tunnels;
	inline messages_handler msgs;
	inline father_handler father;
	inline routes_handler routes;
	inline tasks_handler tasks;
	inline database_handler db;
	inline nodes_handler nodes;
	inline inbox_handler inbox;

	inline void check(void) {
		dadreqs.check(); father.check(); routes.check();
		tunnels.check(); clients.check(); nodes.check();
		msgs.check(); tasks.check();
	}
}

#endif