/**
*	message.cpp - Модуль отвечающий за объекты
*	сообщений децентрализованной сети tin.
*
*	@mrrva - 2019
*/
#include "include/message.hpp" 
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	tinmsg::tinmsg - Конструктор класса tinmsg.
*	Заполняет переменные и указатели.
*
*	@bytes - Байтовый массив.
*/
tinmsg::tinmsg(unsigned char *bytes)
{
	this->bytes[0] = 0x00;

	if (bytes == nullptr || bytes[0] == 0x00
		|| bytes[0] == 0x10)
		return;

	this->length = this->length_detect(bytes);
	memcpy(this->bytes, bytes, this->length);
}
/**
*	tinmsg::is_header_only - Является ли сообщение
*	системным.
*/
bool tinmsg::is_header_only(void)
{
	if (this->bytes[0] == 0x00)
		return false;

	if (this->length == HEADERSIZE)
		return true;
	return false;
}
/**
*	tinmsg::to_bytes - Возвращает сообщение в байтовом
*	формате.
*
*	@len - Количество байт.
*/
unsigned char *tinmsg::to_bytes(size_t &len)
{
	unsigned char *temp;

	if (this->bytes[0] == 0x00)
		return nullptr;

	temp = new unsigned char[HASHSIZE];
	memcpy(temp, this->bytes, TEXTSIZE);
	len = this->length;

	return temp;
}
/**
*	tinmsg::str_key - Перводит публичный ключ пользователя
*	в байтовый формат.
*/
string tinmsg::str_key(void)
{
	unsigned char *bytes;
	string pub_key;

	bytes = this->byte_key();

	if (bytes[0] == 0x00)
		return "";

	pub_key = bin2hex<HASHSIZE>(bytes);

	delete[] bytes;
	return pub_key;
}
/**
*	tinmsg::get_info - Возвращает байтовый массив
*	дополнительной информации сообщения.
*/
unsigned char *tinmsg::get_info(void)
{
	size_t move = 1 + HASHSIZE;
	unsigned char *temp;

	 if (this->bytes[0] == 0x00)
	 	return nullptr;

	temp = new unsigned char[INFOSIZE];
	memcpy(temp, this->bytes + move, INFOSIZE);
	return temp;
}
/**
*	tinmsg::get_info - Возвращает публичный ключ
*	в байтовом формате.
*/
unsigned char *tinmsg::byte_key(void)
{
	unsigned char *temp;

	if (this->bytes[0] == 0x00)
		return nullptr;

	temp = new unsigned char[HASHSIZE];
	memcpy(temp, this->bytes + 1, HASHSIZE);

	return temp;
}
/**
*	tinmsg::is_node - Является ли сообщение от
*	ноды.
*/
bool tinmsg::is_node(void)
{
	if (this->bytes[0] >= 0x10)
		return true;
	return false;
}
/**
*	tinmsg::header_type - Возвращает тип сообщения.
*/
enum tin_htype tinmsg::header_type(void)
{
	enum tin_htype type = INDEFINITE_MESSAGE;
	unsigned char b = this->bytes[0];

	if ((b > 0x00 && b <= 0x06) || (b > 0x10
		&& b <= 0x1d))
		type = static_cast<enum tin_htype>(b);
	return type;
}
/**
*	tinmsg::length_detect - Функция автоматического
*	вычисления длины сообщения.
*
*	@buffer - Байтовый массив.
*/
size_t tinmsg::length_detect(unsigned char *buffer)
{
	if (buffer[0] == 0x05 || buffer[0] == 0x06
		|| buffer[0] >= 0x17)
		return FULLSIZE;
	return HASHSIZE;
}
/**
*	tinmsg::operator = - Оператор присвоения объекта.
*
*	@from - Входящий параметр оператора.
*/
void tinmsg::operator =(tinmsg &from)
{
	unsigned char *bytes;
	size_t len;

	bytes = from.to_bytes(len);

	if (bytes == nullptr || bytes[0] == 0x00
		|| len < HEADERSIZE)
		return;

	memcpy(this->bytes, bytes, len);
	this->length = len;
	delete[] bytes;
}
/**
*	tinmsg::info_nodes - Функция считывания данных
*	из блока сообщения.
*/
unsigned char *tinmsg::garlic_msg(void)
{
	unsigned char *temp;

	if (this->length == FULLSIZE
		|| this->bytes[0] == 0x00)
		return nullptr;

	temp = new unsigned char[TEXTSIZE];
	memcpy(temp, this->bytes + HEADERSIZE, TEXTSIZE);

	return temp;
}
/**
*	tinmsg::info_nodes - Функция считывания данных
*	из блока инфорсвции.
*/
unsigned char *tinmsg::info_msg(void)
{
	unsigned char *temp;

	if (this->bytes[0] == 0x00)
		return nullptr;

	temp = new unsigned char[INFOSIZE];
	memcpy(temp, this->bytes + HASHSIZE + 1, INFOSIZE);

	return temp;
}
/**
*	tinmsg::info_nodes - Функция считывания данных
*	нод в блоке информации сообщения.
*/
map<unsigned char *, string> tinmsg::info_nodes(void)
{
	unsigned char *s_ptr = this->bytes + HASHSIZE + 1;
	size_t parts = INFOSIZE / (HASHSIZE + 4);
	map<unsigned char *, string> data;
	typedef unsigned char us;
	unsigned char *h, b[4];
	string ip;

	if (bytes_sum<INFOSIZE>(s_ptr) == 0x00)
		return data;

	for (size_t i = 0; i < parts; i++) {
		h = new unsigned char[HASHSIZE];

		memcpy(b, s_ptr + HASHSIZE, 4);
		memcpy(h, s_ptr, HASHSIZE);
		ip = ipfrombytes(b);

		if (bytes_sum<HASHSIZE>(h) == 0x00
			|| ip == "0.0.0.0") {
			delete[] h;
			break;
		}

		data.insert(pair<us *, string>(h, ip));
		s_ptr += HASHSIZE + 4;
	}

	return data;
}
/**
*	tinmsg::info_neighbors - Функция считывания данных
*	соседей в блоке информации сообщения.
*/
vector<unsigned char *> tinmsg::info_neighbors(void)
{
	unsigned char *s_ptr = this->bytes + HASHSIZE + 1;
	size_t parts = INFOSIZE / HASHSIZE;
	vector<unsigned char *> buffer;
	unsigned char *tmp;

	if (bytes_sum<INFOSIZE>(s_ptr) == 0x00)
		return buffer;

	for (size_t i = 0; i < parts; i++) {
		tmp = new unsigned char[HASHSIZE];
		memcpy(tmp, s_ptr, HASHSIZE);

		if (bytes_sum<HASHSIZE>(tmp) == 0x00) {
			delete[] tmp;
			break;
		}

		buffer.push_back(tmp);
		s_ptr += HASHSIZE;
	}

	return buffer;
}
/**
*	tinmsg::info_find - Функция считывания данных
*	поиска клиента в блоке информации сообщения.
*/
struct find_request tinmsg::info_find(void)
{
	unsigned char *s_ptr = this->bytes + HASHSIZE + 1;
	struct find_request buffer;
	string target, from;

	if (bytes_sum<INFOSIZE>(s_ptr) == 0x00)
		return buffer;

	buffer.from = ipfrombytes(s_ptr + HASHSIZE + 4 + 1);
	buffer.target = ipfrombytes(s_ptr + HASHSIZE + 1);
	buffer.found = (*s_ptr == 0x00) ? false : true;
	memcpy(buffer.hash, s_ptr + 1, HASHSIZE);

	return buffer;
}
/**
*	tinmsg::info_garlic - Функция считывания данных
*	чесночной маршрутизации.
*/
pair<unsigned char *, unsigned char *> tinmsg::info_garlic(void)
{
	typedef pair<unsigned char *, unsigned char *> pr;
	unsigned char *s_ptr = this->bytes + HASHSIZE + 1;
	pair<unsigned char *, unsigned char *> buffer;
	unsigned char *tmp_1, *tmp_2;

	if (bytes_sum<INFOSIZE>(s_ptr) == 0x00) {
		buffer = pr(nullptr, nullptr);
		return buffer;
	}

	tmp_1 = new unsigned char[HASHSIZE];
	tmp_2 = new unsigned char[HASHSIZE];

	memcpy(tmp_1, s_ptr, HASHSIZE);
	memcpy(tmp_2, s_ptr + HASHSIZE, HASHSIZE);
	buffer = pr(tmp_1, tmp_2);

	return buffer;
}
