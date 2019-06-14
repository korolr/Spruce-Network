/**
*	message.hpp - Заголовочный файл децентрализованной
*	сети TGN. Здесь опублекованны все константы,
*	прототипы, классы и структуры класса tgnmsg.
*
*	@mrrva - 2019
*/
#ifndef TGN_MESSAGE
#define TGN_MESSAGE
/**
*	Главный заголовочный файл проекта.
*/
#include "tgn.hpp"
#include "struct.hpp"
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

enum tgn_garlic {
	/**
	*	Не транспортные типы.
	*/
	G_EMPTY_STATUS		= 0x00,
	G_REQUEST_FIND		= 0x01,
	/**
	*	Успешные типы передачи.
	*/
	G_SUCCESS_SERVER	= 0x02,
	G_SUCCESS_TARGET	= 0x03,
	/**
	*	Ошибки в передаче.
	*/
	G_ERROR_ROUTE		= 0x04,
	G_ERROR_TARGET		= 0x05
};

struct tgn_find_req {
	unsigned char hash[HASHSIZE];
	std::string owner, from;
};

struct tgn_garlic_req {
	unsigned char from[HASHSIZE];
	unsigned char to[HASHSIZE];
	enum tgn_garlic status;
};
/**
*	Классы модуля.
*/
class tgnmsg {
	private :
		unsigned char bytes[FULLSIZE];
		size_t length;

		size_t length_detect(unsigned char *);

	public :
		std::map<unsigned char *, std::string> info_nodes(void);
		std::vector<unsigned char *> info_neighbors(void);
		struct tgn_garlic_req info_garlic(void);
		tgnmsg(unsigned char *bytes = nullptr);
		struct tgn_find_req info_find(void);
		enum tgn_htype header_type(void);
		unsigned char *to_bytes(size_t &);
		unsigned char *garlic_msg(void);
		unsigned char *info_msg(void);
		unsigned char *get_info(void);
		unsigned char *byte_key(void);
		void operator =(tgnmsg &);
		std::string str_key(void);
		bool is_header_only(void);
		bool client_valid(void);
		bool node_valid(void);
		bool is_node(void);
};
/**
*	Вспомогательные шаблоны и функции модуля.
*/
template<bool H>
unsigned char *msg_tmp(enum tgn_htype type)
{
	size_t size = (H) ? HEADERSIZE : FULLSIZE;
	unsigned char *buffer, *key;

	buffer = new unsigned char[size];
	key = tgnstruct::public_key;
	size -= HASHSIZE + 1;

	buffer[0] = static_cast<unsigned char>(type);
	memset(buffer + HASHSIZE + 1, 0x00, size);
	memcpy(buffer + 1, key, HASHSIZE);

	return buffer;
}

template<bool H>
unsigned char *msg_usr(enum tgn_htype type)
{
	unsigned char b = U_RESPONSE_NODES;
	b = static_cast<unsigned char>(type);

	if (b % 2 == 0)
		return nullptr;

	return msg_tmp<H>(static_cast<enum tgn_htype>(++b));
}

template<short S>
std::string bin2hex(unsigned char *bytes)
{
	char key[TEXTSIZE];

	if (S > TEXTSIZE)
		return "";

	memset(key, 0x00, TEXTSIZE);
	sodium_bin2hex(key, TEXTSIZE, bytes, S);
	return std::string(key);
}

template<short S>
unsigned char *hex2bin(std::string line)
{
	unsigned char *buff;
	size_t len;

	if (line.length() % 2 != 0)
		return nullptr;

	len = S / 2 + 1;
	buff = new unsigned char[len];

	sodium_hex2bin(buff, len, line.c_str(),
		line.length(), NULL, NULL, NULL);
	return buff;
}

#endif