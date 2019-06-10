/**
*	network.hpp - Заголовочный файл децентрализованной
*	сети TIN. Здесь опублекованны все константы,
*	прототипы, классы и структуры модуля network.
*
*	@mrrva - 2019
*/
#ifndef TIN_NETWORK
#define TIN_NETWORK
/**
*	Заголовочные файлы других модулей.
*/
#include "tin.hpp"
#include "storage.hpp"
#include "struct.hpp"
#include "router.hpp"
/**
*	Доступные структуры.
*/
struct sockaddr_arr {
	struct sockaddr_in srv;
	struct sockaddr *p_srv;
};
/**
*	Классы модуля.
*/
class _socket {
	private :
		struct sockaddr_arr s_data;
		struct timeval timeout;
		bool stop;

		void send_task(struct tin_task &);
		void thread_recv(void);
		void thread_send(void);
		void set_opts(int &);

	public :
		bool start(void);
		~_socket(void);
		_socket(void);
};

class _requests {
	private :
		std::mutex tasks_mutex;

		void socket_response(unsigned char *, struct sockaddr_in &);
		void thr_client(tinmsg, struct sockaddr_in);
		void thr_node(tinmsg, struct sockaddr_in);
		void response_thr(struct tin_task);

	public :
		void operator <<(struct tin_task);
};
/**
*	Вспомогательные шаблоны и функции модуля.
*/
inline struct tin_ipport ipport_get(struct sockaddr_in a)
{
	struct tin_ipport ipport;
	char temp[21];

	inet_ntop(AF_INET, &a.sin_addr, temp, 20);
	ipport.port = ntohs(a.sin_port);
	ipport.ip = std::string(temp);

	return ipport;
}

inline struct sockaddr_in saddr_get(std::string ip, short p)
{
	struct sockaddr_in sockaddr;

	if (ip.length() < 5)
		return sockaddr;

	sockaddr.sin_addr.s_addr = inet_addr(ip.c_str());
	sockaddr.sin_port = htons(p);
	return sockaddr;
}
/**
*	tinnetwork - Пространство имен модуля network. C помощью
*	него происходит управление модулем network.
*
*	@send - Объект управления потоком отправки сообщений.
*	@recv - Объект управления потоком приема сообщений.
*	@requests - Объект управления новыми запросами.
*	@socket - Объект управления сокетом.
*	@sok - Сокет приема/передачи данных.
*/
namespace tinnetwork {
	inline std::thread send, recv;
	inline _requests requests;
	inline _socket socket;
	inline int sok;
}

#endif