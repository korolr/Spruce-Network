/**
*	encruption.hpp - Заголовочный файл децентрализованной
*	сети TGN. Здесь опублекованно пространство имен
*	модуля encryption.
*
*	@mrrva - 2019
*/
#ifndef TGN_ENCRYPTION
#define TGN_ENCRYPTION
/**
*	Главный заголовочный файл проекта.
*/
#include "tgn.hpp"
#include "struct.hpp"
/**
*	tgnencryption - Пространство имен модуля enctyption.
*	C помощью него происходит управление модулем enctyption.
*
*	@unpack - Функция дешифрации сообщения.
*	@new_keys - Функция генерации новой пары ключей.
*	@pack - Функция шифрации сообщения.
*/
namespace tgnencryption {
	unsigned char *pack(unsigned char *, unsigned char *);
	unsigned char *unpack(unsigned char *);
	void new_keys(void);
}

#endif