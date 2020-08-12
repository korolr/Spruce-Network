
#ifndef _SPRUCE_STORAGE_
#define _SPRUCE_STORAGE_

#include "spruce.hpp"
#include "pack.hpp"

template<typename T, typename S> void
vector_push(T &vec, S &data) {
	auto comp = [](S prev, S next) {
		return prev.id < next.id;
	};

	auto it = lower_bound(vec.begin(), vec.end(),
						  data, comp);
	vec.insert(it, data);
}

template<typename T> bool
vector_search(typename vector<T>::iterator &iter,
			  vector<T> &vec, unsigned char *h, bool debug = false) {

	auto comp = [&](T prev, size_t next) {
		return prev.id < next;
	};

	typename vector<T>::iterator it = vec.begin();
	size_t sum;

	assert(h);

	if (vec.empty()) {
		return false;
	}

	sum = byte_sum(h, HASHSIZE);

	for (;; it++) {
		it = lower_bound(it, vec.end(), sum, comp);

		if (it == vec.end() || it->id != sum) {
			return false;
		}

		if (it->id == sum && HASHEQ(it->hash, h)) {
			break;
		}
	}

	iter = it;
	return true;
}

class tasks_handler {
	private:
		mutex mute;

	public:
		void add(pack, struct sockaddr_in, unsigned char *h = nullptr);
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
		struct client get_notdad(unsigned char *h = nullptr);
		void add_replace(unsigned char *, string);
		struct client nearest(unsigned char *);
		void insert(unsigned char *, string);
		void add(unsigned char *, string);
		void add_attempts(unsigned char *);
		void sub_attempts(unsigned char *);
		void rm_hash(unsigned char *);
		size_t size(void);
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
		vector<struct haship> veclist(void);
		vector<struct client> users(void);
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
		struct haship haship(void);
		void set(struct client);
		void no_father(void);
		void check(void);
#if defined(DEBUG) && DEBUG == true
		void print(void);
#endif
};

class routes_handler {
	private:
		mutex mute;

	public:
		void add(unsigned char *, size_t, enum route_state s = PROGRESS);
		bool upd(unsigned char *, size_t, struct haship);
		bool find(unsigned char *, struct route &);
		void ping(unsigned char *);
		void check(void);
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

class ports_handler {
	private:
		struct sddr_structs st;
		mutex mute;

		bool sys_free(size_t);
		bool is_free(size_t);
		size_t free(void);

	public:
		bool find(unsigned char *, struct tcp_port &);
		bool reg(size_t, unsigned char *);
		size_t try_reg(unsigned char *);
		bool exists(unsigned char *);
		void rm_port(size_t);
		void check(void);
};

class tunnels_handler {
	private:
		mutex mute;

		void port_req(unsigned char *, struct ipport);
		void msg_request(struct tunnel *);
		void sync(struct tunnel *);

	public:
		void user_create(unsigned char *, enum tcp_role,  size_t c = 0, struct haship hi = {});
		void node_create(struct client, enum tcp_role, size_t, struct haship);
		bool find_code(size_t, enum tcp_role, struct tunnel **);
		bool find_hash(unsigned char *, struct tunnel **);
		bool exists_n(enum tcp_role,  size_t);
		bool exists_u(unsigned char *);
		void check(void);
};

namespace storage {
	inline tunnels_handler		tunnels;
	inline clients_handler		clients;
	inline father_handler		father;
	inline routes_handler		routes;
	inline tasks_handler		tasks;
	inline nodes_handler		nodes;
	inline ports_handler		ports;
	inline db_handler			db;

	inline void check(void) {
		father.check(); routes.check(); ports.check();
		clients.check(); tasks.check(); nodes.check();
		tunnels.check();
	}
}

#endif