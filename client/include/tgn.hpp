
#ifndef TGN_LIB
#define TGN_LIB

#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <mutex>
#include <map>
#include <string>
#include <cstring>
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
/**
*	Глобальные шаблоны и функции.
*/
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