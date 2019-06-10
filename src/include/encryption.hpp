/**
*	encruption.hpp - Заголовочный файл децентрализованной
*	сети TIN. Здесь опублекованно пространство имен
*	модуля encryption.
*
*	@mrrva - 2019
*/
#ifndef TIN_ENCRYPTION
#define TIN_ENCRYPTION
/**
*	Главный заголовочный файл проекта.
*/
#include "tin.hpp"
#include "struct.hpp"
/**
*	tinencryption - Пространство имен модуля enctyption.
*	C помощью него происходит управление модулем enctyption.
*
*	@unpack - Функция дешифрации сообщения.
*	@new_keys - Функция генерации новой пары ключей.
*	@pack - Функция шифрации сообщения.
*/
namespace tinencryption {
	unsigned char *pack(unsigned char *, unsigned char *);
	unsigned char *unpack(unsigned char *);
	void new_keys(void);
}

#endif