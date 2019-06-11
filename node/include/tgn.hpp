/**
*	tgn.hpp - Заголовочный файл децентрализованной
*	сети TGN. Здесь опублекованны все константы,
*	прототипы и структуры проекта.
*
*	@mrrva - 2019
*/
#ifndef TGN_NODE
#define TGN_NODE

#include <iostream>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <mutex>
#include <map>
#include <chrono>
#include <string>
#include <cstring>
#include <ctime>
#include <sstream>
#include <sqlite3.h>
#include <sodium.h>
#define TGN_DEBUG 1;
/**
*	Константы проекта.
*/
const short HEADERSIZE	= 500;
const short TEXTSIZE	= 9000;
const short HASHSIZE	= 32;
const short FULLSIZE	= TEXTSIZE + HEADERSIZE;
const short INFOSIZE	= HEADERSIZE - HASHSIZE - 1;
const short PORT		= 2121;
const short TIMEOUT		= 6;
const size_t MIN_NODES	= 50;
/**
*	Глобальные классы и типы данных.
*/
#include "message.hpp"
/**
*	Глобальные шаблоны и функции.
*/
template<short L>
unsigned char bytes_sum(unsigned char *B)
{
	unsigned char summ = 0x00;
	short i;

	if (!B || B == nullptr)
		return summ;

	for (i = 0; i < L; i++)
		summ += B[i];
	return summ;
}

template<short L>
void print_bytes(unsigned char *B)
{
	if (!B || B == nullptr)
		return;

	for (short i = 0; i < L; i++)
		std::cout << "0x" << std::hex << + B[i] << " ";
}

inline std::string ipfrombytes(unsigned char *b)
{
	std::stringstream ss;
	int sum = 0;

	if (!b || b == nullptr)
		return "";

	for (size_t i = 0; i < 4; i++) {
		if (b[i] < 0 || b[i] > 254)
			return "";

		sum += static_cast<int>(b[i]);
	}

	if (sum == 0 || b[0] == 0x00)
		return "";

	ss << static_cast<int>(b[0]) << "."
		<< static_cast<int>(b[1]) << "."
		<< static_cast<int>(b[2]) << "."
		<< static_cast<int>(b[3]);

	return ss.str();
}

inline unsigned char *iptobytes(std::string ip)
{
	std::istringstream ss(ip);
	typedef unsigned char uc;
	std::string token;
	size_t i = 0, tmp;
	unsigned char *b;

	if (ip.length() < 6)
		return nullptr;

	b = new unsigned char[4];

	while (std::getline(ss, token, '.')) {
		if (i > 4)
			break;

		if ((tmp = std::stoi(token)) > 254) {
			delete[] b;
			return nullptr;
		}

		b[i++] = static_cast<uc>(tmp);
	}

	return b;
}

#endif