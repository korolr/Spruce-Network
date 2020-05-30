
#ifndef _SPRUCE_MAIN_
#define _SPRUCE_MAIN_

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
#define UDP_PACK	HASHSIZE * 4 + 8
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
	INDEFINITE		= 0x33,
	DOS				= 0x00,
	REQ_ROLE		= 0x31,
	RES_ROLE		= 0x32,
	/**
	*	Types of user package.
	*/
	USER_REQ_PING	= 0x01,
	USER_RES_PING	= 0x02,

	USER_REQ_TUNNEL	= 0x03,
	USER_RES_TUNNEL	= 0x04,

	USER_REQ_FATHER	= 0x05,
	USER_RES_FATHER	= 0x06,

	USER_REQ_NODE	= 0x07,
	USER_RES_NODE	= 0x08,

	USER_END		= 0x09,
	/**
	*	Types of node package.
	*/
	NODE_REQ_PING	= 0x11,
	NODE_RES_PING	= 0x12,

	NODE_REQ_TUNNEL	= 0x13,
	NODE_RES_TUNNEL	= 0x14,

	NODE_REQ_FATHER	= 0x15,
	NODE_RES_FATHER = 0x16,

	NODE_REQ_FIND	= 0x19,
	NODE_RES_FIND	= 0x1a,

	NODE_REQ_ECHO	= 0x1b, // Echo find. If NODE_REQ_FIND doesn't work correct - send NODE_REQ_EFIND 
	NODE_RES_ECHO	= 0x1c,

	NODE_REQ_NODE	= 0x1d,
	NODE_RES_NODE	= 0x1e,

	NODE_END		= 0x1f
};

struct sddr_structs {
	struct sockaddr_in sddr;
	struct sockaddr *ptr;

	sddr_structs(void) {
		ptr = reinterpret_cast<struct sockaddr *>(&sddr);
	}
};

struct ipport {
	size_t port;
	string ip;
};

struct client {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> ping;
	struct ipport ipp;
	bool is_node;
};

struct udp_father {
	struct client info;
	bool status;
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
	enum udp_type type;
	size_t cookie;
};

struct dadreq {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> time;
	struct sockaddr_in sddr;
};

struct route {
	// father & ipp - Node info
	// hash - client info
	unsigned char father[HASHSIZE], hash[HASHSIZE];
	time_point<system_clock> time;
	struct ipport ipp;
	bool full;
};

struct recvmsg {
	time_point<system_clock> time;
	string hash, file;
	size_t id;
};

enum udp_role : unsigned char {
	UDP_NONE		= 0x00,
	UDP_NODE		= 0x01,
	UDP_USER		= 0x02
};

struct tcp_message {
	time_point<system_clock> time;
	unsigned char hash[HASHSIZE];
	size_t attempts, id;

	struct {
		unsigned char *buff;
		size_t size, ssize;
		string name;
	} data;

	bool done;
};

enum tcp_status : unsigned char {
	NONE			= 0x00,
	TUNNEL_1		= 0x01,
	TUNNEL_2		= 0x02,
	TUNNEL_3		= 0x03,

	CUR_STATUS		= 0x04,
	WAIT_SEND		= 0x05,

	ERROR_D			= 0x06, // Sending is done, just close connection.
	ERROR_S			= 0x07, // Status error, it wasn't updated more than 15s.
	ERROR_T			= 0x08  // Tunnel error, if one of 2 threads isn't working.
};

enum tcp_role : unsigned char {
	TCP_SENDER		= 0x00,
	TCP_TARGET		= 0x04,
	TCP_BINDER1		= 0x01,
	TCP_BINDER2		= 0x02,
	TCP_UBINDER		= 0x03 // If users are connected to the same node. binder1 + binder2
};

#include "pack.hpp"
#include "tcptunnel.hpp"

struct tunnel {
	unsigned char target[HASHSIZE], from[HASHSIZE];
	unsigned char content[PACKLEN];
	time_point<system_clock> time;
	enum tcp_status status;
	tcp_tunnel send, recv;
	size_t length;
	mutex mute;

	tunnel(void) : status(NONE), length(0) { }

	pair<size_t, size_t> ports(void) {
		return make_pair(send.s_port, recv.r_port);
	}

	bool work(void) {
		return send.work || recv.work;
	}

	enum tcp_role role(void) {
		return (send.work) ? send.role : recv.role;
	}
};

namespace structs {
	inline atomic<size_t>				threads = 0;
	inline vector<struct tcp_message>	msgs;
	inline vector<struct tunnel *>		tunnels;
	inline vector<struct udp_task>		tasks;
	inline vector<struct client>		clients;
	inline vector<struct dadreq>		dadreqs;
	inline vector<struct recvmsg>		inbox;
	inline vector<struct client>		nodes;
	inline vector<struct route>			routes;
	inline struct udp_father			father;
	inline struct keypair				keys;
	inline enum udp_role				role;
}

bool
is_null(unsigned char *, size_t);

string
bin2hex(size_t, unsigned char *);

struct ipport
ipport_get(struct sockaddr_in);

string
bytes2ip(unsigned char *);

unsigned char *
hex2bin(size_t, string);

struct sockaddr_in
sddr_get(struct ipport);

size_t
random_cookie(void);

unsigned char *
ip2bytes(string);

void
socket_close(int);

#define THREAD_START() if (structs::threads >= PACKLIM) \
	return; ++structs::threads;

#define THREAD_END() --structs::threads; return;

#endif