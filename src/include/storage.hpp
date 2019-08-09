/**
*	storage.hpp - Заголовочный файл децентрализованной
*	сети TGN. Здесь опублекованны все константы,
*	прототипы, классы и структуры модуля storage.
*
*	@mrrva - 2019
*/
#ifndef TGN_STORAGE
#define TGN_STORAGE
/**
*	Заголовочные файлы других модулей.
*/
#include "tgn.hpp"
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
		void set_node(struct tgn_node);
		std::string get_var(std::string);
		void remove_node(std::string);
		void remove_var(std::string);
		void ping_node(std::string);
		~_database(void);
		_database(void);
};

class _nodes {
	private :
		time_point<system_clock> last_req;
		std::mutex mute;

	public :
		bool find_hash(struct tgn_node &, unsigned char *);
		bool find_ip(struct tgn_node &, std::string);
		struct tgn_node get_last(void);
		void ping(struct sockaddr_in &);
		void request_getlist(void);
		bool add(struct tgn_node);
		void remove(std::string);
		void autocheck(void);
		void select(void);
		_nodes(void);
};

class _clients {
	private :
		std::mutex mute;

	public :
		void update(unsigned char *, struct tgn_ipport &);
		bool find(struct tgn_client &, unsigned char *);
		bool exists(unsigned char *);
		void autoremove(void);
};

class _tasks {
	private :
		std::mutex mute;

	public :
		void add(struct tgn_task);
		void remove_first(void);
};

class _neighbors {
	private :
		void new_requests(void);

		time_point<system_clock> last_req;
		std::mutex mute;

	public :
		bool find(struct tgn_neighbor &, unsigned char *);
		void add(unsigned char *, unsigned char *);
		std::vector<time_list> timelist(void);
		bool exists(unsigned char *);
		void clear(unsigned char *);
		void autocheck(void);
		_neighbors(void);
};

class _routes {
	private :
		std::mutex mute;

	public :
		void add(unsigned char *, struct tgn_ipport, bool find = true);
		void update(unsigned char *, struct tgn_ipport);
		bool find(struct tgn_route &, unsigned char *);
		void add(unsigned char *, std::string);
		void remove_hash(unsigned char *);
		size_t exists(unsigned char *);
		void autoremove(void);
};

class _garlic {
	private :
		void task_error(struct tgn_garlic, struct tgn_client);

		std::mutex mute;

	public :
		void set_status(unsigned char *, unsigned char *, enum tgn_status);
		bool find(struct tgn_garlic &, unsigned char *, unsigned char *);
		void remove(struct tgn_garlic &);
		bool exists(struct tgn_garlic);
		void add(struct tgn_garlic);
		void autoremove(void);
};
/**
*	tgnstorage - Пространство имен модуля storage. C помощью
*	него происходит управление модулем storage.
*
*	@neighbors - Объект управления структуры соседных клиентов.
*	@clients - Объект управления структуры клиентов.
*	@db - Объект управления базой данных.
*	@routes - Объект управления структурой маршрутов.
*	@garlic - Объект управления сткруктурой статусов сообщений.
*	@clients - Объект управления структуры нод.
*	@tasks - Объект управления заданиями.
*/
namespace tgnstorage {
	inline _neighbors	neighbors;
	inline _clients		clients;
	inline _routes		routes;
	inline _garlic		garlic;
	inline _nodes		nodes;
	inline _tasks		tasks;
	inline _database	db;
}

#endif