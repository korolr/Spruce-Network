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
*	@public_key - Публичный ключ ноды.
*	@secret_key - Секретный ключ ноды.
*/
namespace tgnstruct {
	inline std::vector<struct tgn_neighbor>	neighbors;
	inline std::vector<struct tgn_client>	clients;
	inline std::vector<struct tgn_route>	routes;
	inline std::vector<struct tgn_node>		nodes;
	inline std::vector<struct tgn_task>		tasks;
	inline unsigned char *public_key;
	inline unsigned char *secret_key;
}

#endif