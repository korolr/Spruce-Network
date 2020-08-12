
#ifndef _SPRUCE_MAIN_
#define _SPRUCE_MAIN_
// upper_bound / sort & function inside / binary_search / constexpr
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <functional>
#include <sqlite3.h>
#include <algorithm>
#include <sodium.h>
#include <unistd.h>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <cstring>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <cstdio>
#include <mutex>
#include <map>

#define HASHSIZE	32
#define INFOSIZE	250 - 65
#define UDP_PACK	HASHSIZE + 5 + INFOSIZE
#define MAX_UDPSZ	UDP_PACK + 200 + crypto_box_SEALBYTES
#define UDP_PORT	2121
#define TIMEOUT		10
#define	MAX_NODES	40
#define NODE_LIMIT	100
#define NODE_MIN	20
#define PACKLEN		2500
#define DEBUG		true
#define API_PORT	2122
#define PACKLIM		200

using namespace std;
using chrono::system_clock;
using chrono::time_point;
/**
*	Global spruce structs.
*
*/
enum udp_type : unsigned char {
	/**
	*	Universal package types.
	*/
	INDEFINITE		= 0x35,
	DOS				= 0x00,
	REQ_ROLE		= 0x31,
	RES_ROLE		= 0x32,
	REQ_FATHER		= 0x33,
	RES_FATHER		= 0x34,
	/**
	*	Types of user package.
	*/
	USER_REQ_PING	= 0x01,			// straight
	USER_RES_PING	= 0x02,

	USER_REQ_TUNNEL	= 0x03,
	USER_RES_TUNNEL	= 0x04,

	USER_REQ_FIND	= 0x05,			// straight
	USER_RES_FIND	= 0x06,

	USER_REQ_MSG	= 0x07,
	USER_RES_MSG	= 0x08,

	USER_REQ_PORT	= 0x09,			// straight
	USER_RES_PORT	= 0x0a,

	USER_END		= 0x0b,
	/**
	*	Types of node package.
	*/
	NODE_REQ_PING	= 0x11,
	NODE_RES_PING	= 0x12,

	NODE_REQ_TUNNEL	= 0x13,
	NODE_RES_TUNNEL	= 0x14,

	NODE_REQ_FIND	= 0x15,
	NODE_RES_FIND	= 0x16,

	NODE_REQ_PORT	= 0x17,
	NODE_RES_PORT	= 0x18,

	NODE_REQ_MSG	= 0x19,
	NODE_RES_MSG	= 0x20,

	NODE_END		= 0x21
};

struct sddr_structs {
	struct sockaddr_in sddr;
	struct sockaddr *ptr;

	sddr_structs(void) {
		ptr = reinterpret_cast<struct sockaddr *>(&sddr);
	}
};

struct hashport {
	unsigned char hash[HASHSIZE];
	size_t port = 0;
};

struct haship {
	unsigned char hash[HASHSIZE];
	string ip;
};

struct ipport {
	size_t port = 0;
	string ip  = "";

	bool operator==(struct ipport &ipp) {
		return ipp.ip == ip;
	}

	bool operator!=(struct ipport &ipp) {
		return ipp.ip != ip;
	}

	ipport(size_t p = 0, string i = "")
		: port(p), ip(i) { };
};

struct client {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> ping;
	struct ipport ipp = {0, ""};
	bool is_node = false;
};

struct client_att : client {
	size_t attempts = 0, id;
};

struct udp_father {
	struct client info;
	atomic<bool> status = false;
};

struct keypair {
	unsigned char *pub;
	unsigned char *sec;
};

struct udp_task {
	time_point<system_clock> time;
	unsigned char buff[MAX_UDPSZ];
	struct sockaddr_in sddr;
	size_t attempts, len;
	// For searching.
	unsigned char hash[HASHSIZE];
	size_t cookie;
	enum udp_type type;
};

enum route_state : unsigned char {
	FOUND			= 0x02,
	NFOUND			= 0x01,
	PROGRESS		= 0x00
};

struct route {
	unsigned char hash[HASHSIZE];
	unsigned char node[HASHSIZE];
	time_point<system_clock> time;
	enum route_state status;
	struct ipport ipp;
	size_t code, id;
};

struct tcp_port {
	time_point<system_clock> time;
	unsigned char hash[HASHSIZE];
	bool confirm = false;
	size_t port, id;
};

enum udp_role : unsigned char {
	UDP_NONE		= 0x00,
	UDP_NODE		= 0x01,
	UDP_USER		= 0x02
};

enum tcp_role : unsigned char {
	TCP_NONE		= 0x00,
	TCP_SENDER 		= 0x01,
	TCP_BINDER1		= 0x02,
	TCP_TARGET		= 0x03,
	TCP_BINDER2		= 0x04
};

class spruceapi {
private:
	static unsigned char *
	tunnel_request(unsigned char *, size_t &);

	static unsigned char *
	find_request(unsigned char *, size_t &);

	static unsigned char *
	spruce_status(size_t &);

public:
	static unsigned char *
	processing(unsigned char *, size_t &);
};

string
bin2hex(size_t, unsigned char *);

struct ipport
ipport_get(struct sockaddr_in);

string
bin2ip(unsigned char *);

unsigned char *
hex2bin(size_t, string);

struct sockaddr_in
sddr_get(struct ipport);

size_t
random_cookie(void);

unsigned char *
ip2bin(string);

int
new_socket(int, size_t);

void
set_sockaddr(struct sockaddr_in &,
			 size_t port = 0,
			 string ip = "");

size_t
byte_sum(unsigned char *, size_t);

#define THREAD_START()												\
	if (structs::threads >= PACKLIM) {								\
		return;														\
	}																\
	++structs::threads;

#define THREAD_END()												\
	--structs::threads;												\
	return;

#define HASHCPY(to, from)											\
	memcpy((to), (from), HASHSIZE);

#define IS_ME(key)													\
	(memcmp((key), structs::keys.pub, HASHSIZE) == 0)

#define IS_FATHER(key)												\
	(memcmp((key), structs::father.info.hash, HASHSIZE) == 0)

#define IS_NULL(ptr, size)											\
	(byte_sum((ptr), (size)) == 0)

#define HASHEQ(h1, h2)												\
	(memcmp((h1), (h2), HASHSIZE) == 0)

#define CLOSE_SOCKET(s)												\
	if ((s) != 0) {													\
		shutdown((s), SHUT_RDWR);									\
		close((s));													\
	}

#include "tcptunnel.hpp"

namespace structs {
	inline atomic<size_t>				threads = 0;
	inline vector<struct client>		clients;
	inline vector<struct tunnel *>		tunnels;
	inline vector<struct tcp_port>		ports;
	inline vector<struct udp_task>		tasks;
	inline vector<struct client_att>	nodes;
	inline vector<struct route>			routes;
	inline struct udp_father			father;
	inline struct keypair				fkeys;
	inline struct keypair				keys;
	inline enum udp_role				role;

	namespace api {
		//inline vector<struct hashport>	inbox;
		//inline vector<struct msg>		msgs;
	}
}

#endif