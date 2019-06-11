/**
*	message.cpp - Модуль отвечающий за объекты
*	сообщений децентрализованной сети TGN.
*
*	@mrrva - 2019
*/
#include "include/message.hpp" 
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	tgnmsg::tgnmsg - Конструктор класса tgnmsg.
*	Заполняет переменные и указатели.
*
*	@bytes - Байтовый массив.
*/
tgnmsg::tgnmsg(unsigned char *bytes)
{
	this->bytes[0] = 0x00;

	if (bytes == nullptr || bytes[0] == 0x00
		|| bytes[0] == 0x10)
		return;

	this->length = this->length_detect(bytes);
	memcpy(this->bytes, bytes, this->length);
}
/**
*	tgnmsg::is_header_only - Является ли сообщение
*	системным.
*/
bool tgnmsg::is_header_only(void)
{
	if (this->bytes[0] == 0x00)
		return false;

	if (this->length == HEADERSIZE)
		return true;
	return false;
}
/**
*	tgnmsg::to_bytes - Возвращает сообщение в байтовом
*	формате.
*
*	@len - Количество байт.
*/
unsigned char *tgnmsg::to_bytes(size_t &len)
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
*	tgnmsg::str_key - Перводит публичный ключ пользователя
*	в байтовый формат.
*/
string tgnmsg::str_key(void)
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
*	tgnmsg::get_info - Возвращает байтовый массив
*	дополнительной информации сообщения.
*/
unsigned char *tgnmsg::get_info(void)
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
*	tgnmsg::get_info - Возвращает публичный ключ
*	в байтовом формате.
*/
unsigned char *tgnmsg::byte_key(void)
{
	unsigned char *temp;

	if (this->bytes[0] == 0x00)
		return nullptr;

	temp = new unsigned char[HASHSIZE];
	memcpy(temp, this->bytes + 1, HASHSIZE);

	return temp;
}
/**
*	tgnmsg::is_node - Является ли сообщение от
*	ноды.
*/
bool tgnmsg::is_node(void)
{
	if (this->bytes[0] >= 0x10)
		return true;
	return false;
}
/**
*	tgnmsg::header_type - Возвращает тип сообщения.
*/
enum tgn_htype tgnmsg::header_type(void)
{
	enum tgn_htype type = INDEFINITE_MESSAGE;
	unsigned char b = this->bytes[0];

	if ((b > 0x00 && b <= 0x06) || (b > 0x10
		&& b <= 0x1d))
		type = static_cast<enum tgn_htype>(b);
	return type;
}
/**
*	tgnmsg::length_detect - Функция автоматического
*	вычисления длины сообщения.
*
*	@buffer - Байтовый массив.
*/
size_t tgnmsg::length_detect(unsigned char *buffer)
{
	if (buffer[0] == 0x05 || buffer[0] == 0x06
		|| buffer[0] >= 0x17)
		return FULLSIZE;
	return HASHSIZE;
}
/**
*	tgnmsg::operator = - Оператор присвоения объекта.
*
*	@from - Входящий параметр оператора.
*/
void tgnmsg::operator =(tgnmsg &from)
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
*	tgnmsg::info_nodes - Функция считывания данных
*	из блока сообщения.
*/
unsigned char *tgnmsg::garlic_msg(void)
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
*	tgnmsg::info_nodes - Функция считывания данных
*	из блока инфорсвции.
*/
unsigned char *tgnmsg::info_msg(void)
{
	unsigned char *temp;

	if (this->bytes[0] == 0x00)
		return nullptr;

	temp = new unsigned char[INFOSIZE];
	memcpy(temp, this->bytes + HASHSIZE + 1, INFOSIZE);

	return temp;
}
/**
*	tgnmsg::info_nodes - Функция считывания данных
*	нод в блоке информации сообщения.
*/
map<unsigned char *, string> tgnmsg::info_nodes(void)
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
*	tgnmsg::info_neighbors - Функция считывания данных
*	соседей в блоке информации сообщения.
*/
vector<unsigned char *> tgnmsg::info_neighbors(void)
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
*	tgnmsg::info_find - Функция считывания данных
*	поиска клиента в блоке информации сообщения.
*/
struct find_request tgnmsg::info_find(void)
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
*	tgnmsg::info_garlic - Функция считывания данных
*	чесночной маршрутизации.
*/
pair<unsigned char *, unsigned char *> tgnmsg::info_garlic(void)
{
	typedef pair<unsigned char *, unsigned char *> pr;
	unsigned char *s_ptr = this->bytes + HASHSIZE + 1;
	unsigned char *to, *from;

	if (bytes_sum<INFOSIZE>(s_ptr) == 0x00)
		return pr(nullptr, nullptr);

	from = new unsigned char[HASHSIZE];
	to = new unsigned char[HASHSIZE];

	memcpy(from, s_ptr + HASHSIZE, HASHSIZE);
	memcpy(to, s_ptr, HASHSIZE);

	return pr(to, from);
}

pair<size_t, unsigned char *> tgnmsg::info_valid(void)
{
	unsigned char *s_ptr = this->bytes + HASHSIZE + 1;
	typedef pair<size_t, unsigned char *> pr;
	pair<size_t, unsigned char *> buffer;
	size_t len = INFOSIZE - 1;
	unsigned char *tmp;

	if (bytes_sum<INFOSIZE>(s_ptr) == 0x00)
		return pr(0, nullptr);

	if (*s_ptr == 0x00) {
		tmp = new unsigned char[len];
		memcpy(tmp, s_ptr + 1, len);

		return pr(1, tmp);
	}

	/* HERE!!!!!!!!!!!!!!!! */
	return pr(2, nullptr)
}