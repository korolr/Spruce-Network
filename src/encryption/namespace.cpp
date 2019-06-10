/**
*	encryption.cpp - Модуль отвечающий за работу с
*	элементами шифрации и идетификации в сети TIN.
*
*	@mrrva - 2019
*/
#include "../include/encryption.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	tinencryption::new_keys - Генерация пары ключей
*	шифрации.
*/
void tinencryption::new_keys(void)
{
	using tinstruct::secret_key;
	using tinstruct::public_key;

	public_key = new unsigned char[HASHSIZE];
	secret_key = new unsigned char[HASHSIZE];

	crypto_box_keypair(public_key, secret_key);
}
/**
*	tinencryption::pack - Шифрация байтового массива
*	для пересылки пользователю.
*
*	@text - Указатель на байтовый массив.
*	@key - Публичный ключ пользователя.
*/
unsigned char *tinencryption::pack(unsigned char *text,
	unsigned char *key)
{
	unsigned char *buffer = nullptr;
	size_t len;

	if (!text || !key) {
		cout << "Error: Incorrect args in "
			<< "tinencryption::pack\n";
		return nullptr;
	}

	len = TEXTSIZE + 100 + crypto_box_SEALBYTES;
	buffer = new unsigned char[len];
	memset(buffer, 0x00, len);

	crypto_box_seal(buffer, text, TEXTSIZE, key);
	return buffer;
}
/**
*	tinencryption::unpack - Дешифрация байтового массива
*	для дальнейшей обработки.
*
*	@text - Указатель на байтовый массив.
*/
unsigned char *tinencryption::unpack(unsigned char *text)
{
	using tinstruct::secret_key;
	using tinstruct::public_key;

	unsigned char *buffer = nullptr;
	size_t len;

	if (!text) {
		cout << "Error: Incorrect args in "
			<< "tinencryption::unpack\n";
		return nullptr;
	}

	len = TEXTSIZE + crypto_box_SEALBYTES;
	buffer = new unsigned char[len];
	memset(buffer, 0x00, len);

	if (crypto_box_seal_open(buffer, text, len
		, public_key, secret_key) != 0) {
		cout << "Error: crypto_box_seal_open in "
			<< "tinencryption::unpack\n";
		return nullptr;
	}

	return buffer;
}