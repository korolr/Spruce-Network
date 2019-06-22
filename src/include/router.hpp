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
		void new_msg(struct tgn_ipport, tgnmsg &, unsigned char *, int);
		void from_neighbors(unsigned char *);
		bool from_clients(unsigned char *);
		void make_find(unsigned char *);

	public :
		unsigned char *status_garlic(tgnmsg &, struct sockaddr_in &);
		unsigned char *client_garlic(tgnmsg &, struct sockaddr_in &);
		unsigned char *node_garlic(tgnmsg &, struct sockaddr_in &);

		unsigned char *req_neighbors(tgnmsg &, struct sockaddr_in &);
		unsigned char *rsp_neighbors(tgnmsg &, struct sockaddr_in &);

		unsigned char *req_find(tgnmsg &, struct sockaddr_in &);
		unsigned char *rsp_find(tgnmsg &, struct sockaddr_in &);

		unsigned char *req_nodes(tgnmsg &, struct sockaddr_in &, bool);
		unsigned char *rsp_nodes(tgnmsg &, struct sockaddr_in &);
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
	unsigned char *garlic_back(struct tgn_garlic, enum tgn_status, int);
	unsigned char *client(tgnmsg, struct sockaddr_in);
	unsigned char *node(tgnmsg, struct sockaddr_in);
}

#endif