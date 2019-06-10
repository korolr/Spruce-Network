/**
*	struct.hpp - Заголовочный файл децентрализованной
*	сети TIN. Здесь опублекованны все главные объекты,
*	структуры и пространство имен с общими данными.
*
*	@mrrva - 2019
*/
#ifndef TIN_STRUCT
#define TIN_STRUCT
/**
*	Главный заголовочный файл проекта.
*/
#include "tin.hpp"
/**
*	Используемые пространства имен.
*/
using std::chrono::system_clock;
using std::chrono::time_point;
/**
*	Доступные структуры.
*/
struct tin_ipport {
	std::string ip;
	size_t port;
};

struct tin_client {
	unsigned char hash[HASHSIZE];
	struct tin_ipport ipport;
	time_point<system_clock> ping;
};

struct tin_node {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> ping;
	std::string ip;
	bool remove;
};

struct tin_neighbor {
	unsigned char client[HASHSIZE];
	unsigned char node[HASHSIZE];
	time_point<system_clock> ping;
};

struct tin_route {
	unsigned char hash[HASHSIZE];
	time_point<system_clock> ping;
	struct tin_ipport ipport;
	bool find;
};

struct tin_task {
	unsigned char bytes[FULLSIZE];
	struct sockaddr_in client_in;
	bool target_only;
	short length;
};
/**
*	tinstruct - Пространство имен общих объектов. C помощью
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
namespace tinstruct {
	inline std::vector<struct tin_neighbor>	neighbors;
	inline std::vector<struct tin_client>	clients;
	inline std::vector<struct tin_route>	routes;
	inline std::vector<struct tin_node>		nodes;
	inline std::vector<struct tin_task>		tasks;
	inline unsigned char *public_key;
	inline unsigned char *secret_key;
}

#endif