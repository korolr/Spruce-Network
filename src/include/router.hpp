/**
*	router.hpp - Заголовочный файл децентрализованной
*	сети TGN. Здесь опублекованны все константы,
*	прототипы, классы и структуры модуля router.
*
*	@mrrva - 2019
*/
#ifndef TGN_ROUTER
#define TGN_ROUTER
/**
*	Заголовочные файлы других модулей.
*/
#include "tgn.hpp"
#include "struct.hpp"
#include "storage.hpp"
/**
*	Классы модуля.
*/
class _router {
	private :
		struct tgn_task cycle_find(unsigned char *, struct sockaddr_in, struct find_request);
		struct tgn_task send_find(unsigned char *, struct find_request, std::string);
		void insert_nodes(std::map<unsigned char *, std::string>);
		struct tgn_task send_message(struct tgn_ipport, tgnmsg &);
		struct tgn_task neighbors_list(struct sockaddr_in &);
		unsigned char *list_nodes(enum tgn_htype);
		void from_neighbors(unsigned char *);
		void from_clients(unsigned char *);
		void make_find(unsigned char *);

	public :
		struct tgn_task su_nodes(tgnmsg &, struct sockaddr_in &, bool);
		struct tgn_task s_neighbors(tgnmsg &, struct sockaddr_in &);
		struct tgn_task u_garlic(tgnmsg &, struct sockaddr_in &);
		struct tgn_task s_garlic(tgnmsg &, struct sockaddr_in &);
		struct tgn_task s_find(tgnmsg &, struct sockaddr_in &);
};
/**
*	Объекты модуля.
*/
inline _router router;
/**
*	tgnrouter - Пространство имен модуля router. C помощью
*	него происходит управление модулем router.
*
*	@client - Функция обработки клиентских маршрутов.
*	@node - Функция обработки маршрутов для нод сети.
*/
namespace tgnrouter {
	struct tgn_task client(tgnmsg, struct sockaddr_in);
	struct tgn_task node(tgnmsg, struct sockaddr_in);
}

#endif