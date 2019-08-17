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
struct tgn_find_req {
	unsigned char hash[HASHSIZE];
	std::string owner, from;
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
		tgnmsg(unsigned char *bytes = nullptr);
		struct tgn_find_req info_find(void);
		struct tgn_garlic info_garlic(void);
		void from_garlic(unsigned char *);
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