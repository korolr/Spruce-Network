/**
*	storage.hpp - Заголовочный файл децентрализованной
*	сети TIN. Здесь опублекованны все константы,
*	прототипы, классы и структуры модуля storage.
*
*	@mrrva - 2019
*/
#ifndef TIN_STORAGE
#define TIN_STORAGE
/**
*	Заголовочные файлы других модулей.
*/
#include "tin.hpp"
#include "struct.hpp"
#include "network.hpp"

using std::chrono::system_clock;
using std::chrono::time_point;
/**
*	Новые типы данных модуля.
*/
typedef std::pair<unsigned char *, time_point<system_clock>> time_list;
/**
*	Классы модуля.
*/
class _database {
	private :
		std::mutex var_mutex, nodes_mutex;
		sqlite3 *db;

		void remove_old_nodes(void);
		void create_tables(void);

	public :
		std::map<std::string, std::string> select_nodes(void);
		void new_node(std::string, std::string);
		void set_var(std::string, std::string);
		void delete_node(unsigned char *);
		void set_node(struct tin_node);
		std::string get_var(std::string);
		void remove_node(std::string);
		void remove_var(std::string);
		void ping_node(std::string);
		~_database(void);
		_database(void);
};

class _nodes {
	private :
		std::mutex mute;

	public :
		bool find_hash(struct tin_node &, unsigned char *);
		bool find_ip(struct tin_node &, std::string);
		struct tin_node get_last(void);
		void ping(struct sockaddr_in &);
		bool add(struct tin_node);
		void remove(std::string);
		void select(void);
};

class _clients {
	private :
		std::mutex mute;

	public :
		void update(unsigned char *, struct tin_ipport &);
		bool find(struct tin_client &, unsigned char *);
		bool exists(unsigned char *);
		void remove(void);
};

class _tasks {
	private :
		std::mutex mute;

	public :
		void remove(std::vector<struct tin_task>::iterator);
		void add(struct tin_task);
};

class _neighbors {
	private :
		std::mutex mute;

	public :
		bool find(struct tin_neighbor &, unsigned char *);
		void add(unsigned char *, unsigned char *);
		std::vector<time_list> timelist(void);
		bool exists(unsigned char *);
		void clear(unsigned char *);
};

class _routes {
	private :
		std::mutex mute;

	public :
		void add(unsigned char *, struct tin_ipport, bool find = true);
		void update(unsigned char *, struct tin_ipport);
		bool find(struct tin_route &, unsigned char *);
		void add(unsigned char *, std::string);
		void remove_hash(unsigned char *);
		size_t exists(unsigned char *);
		void remove(void);
};
/**
*	tinstorage - Пространство имен модуля storage. C помощью
*	него происходит управление модулем storage.
*
*	@neighbors - Объект управления структуры соседных клиентов.
*	@clients - Объект управления структуры клиентов.
*	@db - Объект управления базой данных.
*	@routes - Объект управления структурой маршрутов.
*	@clients - Объект управления структуры нод.
*	@tasks - Объект управления заданиями.
*/
namespace tinstorage {
	inline _neighbors	neighbors;
	inline _clients		clients;
	inline _routes		routes;
	inline _nodes		nodes;
	inline _tasks		tasks;
	inline _database	db;
}

#endif