/**
*	struct.hpp - Заголовочный файл децентрализованной
*	сети TGN. Здесь опублекованны все главные объекты,
*	структуры и пространство имен с общими данными.
*
*	@mrrva - 2019
*/
#ifndef TGN_STRUCT
#define TGN_STRUCT
/**
*	Главный заголовочный файл проекта.
*/
#include "tgn.hpp"
/**
*	Используемые пространства имен.
*/
using std::chrono::system_clock;
using std::chrono::time_point;
/**
*	Доступные структуры.
*/
enum tgn_htype {
	INDEFINITE_MESSAGE	= 0x99,
	/**
	*	Клиентские типы сообщений.
	*/
	U_REQUEST_DOS		= 0x00,
	U_RESPONSE_DOS		= 0x00,

	U_REQUEST_NODES		= 0x01,
	U_RESPONSE_NODES	= 0x02,

	U_REQUEST_PING		= 0x03,
	U_RESPONSE_PING		= 0x04,

	U_REQUEST_GARLIC	= 0x05,
	U_RESPONSE_GARLIC	= 0x06,

	U_REQUEST_VALID		= 0x07,
	U_RESPONSE_VALID	= 0x08,
	/**
	*	Серверные типы сообщений.
	*/
	S_REQUEST_DOS		= 0x10,
	S_RESPONSE_DOS		= 0x10,

	S_REQUEST_NODES		= 0x11,
	S_RESPONSE_NODES	= 0x12,

	S_REQUEST_CLIENTS	= 0x13,
	S_RESPONSE_CLIENTS	= 0x14,

	S_REQUEST_FIND		= 0x15,
	S_RESPONSE_FIND		= 0x16,

	S_REQUEST_GARLIC	= 0x17,
	S_RESPONSE_GARLIC	= 0x18,

	S_REQUEST_VALID		= 0x1e,
	S_RESPONSE_VALID	= 0x1f
};

enum tgn_status {
	EMPTY_STATUS		= 0x00,
	REQUEST_FIND		= 0x01,

	GOOD_SERVER			= 0x02,
	GOOD_TARGET			= 0x03,

	ERROR_ROUTE			= 0x04,
	ERROR_TARGET		= 0x05
};

struct tgn_ipport {
	std::string ip;
	size_t port;
};

struct tgn_client {
	unsigned char hash[HASHSIZE];
	struct tgn_ipport ipport;
	time_point<system_clock> ping;
};

struct tgn_node {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> ping;
	std::string ip;
	bool remove;
};

struct tgn_neighbor {
	unsigned char client[HASHSIZE];
	unsigned char node[HASHSIZE];
	time_point<system_clock> ping;
};

struct tgn_route {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> ping;
	struct tgn_ipport ipport;
	bool find;
};

struct tgn_garlic {
	unsigned char from[HASHSIZE];
	unsigned char to[HASHSIZE];
	time_point<system_clock> ping;
	enum tgn_status status;
};

struct tgn_task {
	unsigned char bytes[FULLSIZE];
	struct sockaddr_in client_in;
	bool target_only;
	short length;
};
/**
*	tgnstruct - Пространство имен общих объектов. C помощью
*	него происходит передача данных между модулями.
*
*	@neighbors - Список клиентов у знакомых нод.
*	@clients - Список всех подключенных клиентов сети.
*	@nodes - Список всех известных нод сети.
*	@routes - Список всех маршрутов сети.
*	@tasks - Список всех заданий ноды.
*	@garlic - Список всех статусов сообщений.
*	@public_key - Публичный ключ ноды.
*	@secret_key - Секретный ключ ноды.
*/
namespace tgnstruct {
	inline std::vector<struct tgn_neighbor>	neighbors;
	inline std::vector<struct tgn_client>	clients;
	inline std::vector<struct tgn_route>	routes;
	inline std::vector<struct tgn_node>		nodes;
	inline std::vector<struct tgn_task>		tasks;
	inline std::vector<struct tgn_garlic>	garlic;
	inline unsigned char *public_key;
	inline unsigned char *secret_key;
}

#endif