/**
*	router.hpp - Заголовочный файл децентрализованной
*	сети TIN. Здесь опублекованны все константы,
*	прототипы, классы и структуры модуля router.
*
*	@mrrva - 2019
*/
#ifndef TIN_ROUTER
#define TIN_ROUTER
/**
*	Заголовочные файлы других модулей.
*/
#include "tin.hpp"
#include "struct.hpp"
#include "storage.hpp"
/**
*	Классы модуля.
*/
class _router {
	private :
		struct tin_task cycle_find(unsigned char *, struct sockaddr_in, struct find_request);
		struct tin_task send_find(unsigned char *, struct find_request, std::string);
		void insert_nodes(std::map<unsigned char *, std::string>);
		struct tin_task send_message(struct tin_ipport, tinmsg &);
		struct tin_task neighbors_list(struct sockaddr_in &);
		unsigned char *list_nodes(enum tin_htype);
		void from_neighbors(unsigned char *);
		void from_clients(unsigned char *);
		void make_find(unsigned char *);

	public :
		struct tin_task su_nodes(tinmsg &, struct sockaddr_in &, bool);
		struct tin_task s_neighbors(tinmsg &, struct sockaddr_in &);
		struct tin_task u_garlic(tinmsg &, struct sockaddr_in &);
		struct tin_task s_garlic(tinmsg &, struct sockaddr_in &);
		struct tin_task s_find(tinmsg &, struct sockaddr_in &);
};
/**
*	Объекты модуля.
*/
inline _router router;
/**
*	tinrouter - Пространство имен модуля router. C помощью
*	него происходит управление модулем router.
*
*	@client - Функция обработки клиентских маршрутов.
*	@node - Функция обработки маршрутов для нод сети.
*/
namespace tinrouter {
	struct tin_task client(tinmsg, struct sockaddr_in);
	struct tin_task node(tinmsg, struct sockaddr_in);
}

#endif