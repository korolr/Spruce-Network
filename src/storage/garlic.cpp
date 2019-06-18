/**
*	garlic.cpp - Модуль отвечающий за работу
*	с список статусов отправленных сообщений.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_garlic::_database - Добавляет новый элемент
*	в общий список.
*
*	@one - Структура данных новой записи.
*/
void _garlic::add(struct tgn_garlic one)
{
	const size_t hs = HASHSIZE;

	if (memcmp(one.from, one.to, hs) == 0
		|| bytes_sum<hs>(one.from) == 0
		|| bytes_sum<hs>(one.to) == 0)
		return;

	one.ping = system_clock::now();

	this->mute.lock();
	tgnstruct::garlic.push_back(one);
	this->mute.unlock();
}
/**
*	_garlic::set_status - Установить новый
*	статус сообщения.
*
*	@from - Хэш отправителя
*	@to - Хэщ получателя.
*	@s - Статус сообщения.
*/
void _garlic::set_status(unsigned char *from,
	unsigned char *to, enum tgn_status s)
{
	using tgnstruct::garlic;

	int cmp_1, cmp_2;

	if (!from || !to || s == EMPTY_STATUS
		|| garlic.size() == 0)
		return;

	this->mute.lock();

	for (auto &p : garlic) {
		cmp_1 = memcmp(p.from, from, HASHSIZE);
		cmp_2 = memcmp(p.to, to, HASHSIZE);

		if (cmp_1 == 0 && cmp_2 == 0) {
			p.ping = system_clock::now();
			p.status = s;
			break;
		}
	}

	this->mute.unlock();
}
/**
*	_garlic::remove - Удаление элемента из
*	общего списка.
*
*	@one - Структура данных для поиска.
*/
void _garlic::remove(struct tgn_garlic &one)
{
	using tgnstruct::garlic;

	vector<struct tgn_garlic>::iterator it;
	int cmp_1, cmp_2, hs = HASHSIZE;

	if (garlic.size() == 0)
		return;

	this->mute.lock();
	it = garlic.begin();

	for (; it != garlic.end(); it++) {
		cmp_1 = memcmp((*it).from, one.from, hs);
		cmp_2 = memcmp((*it).to, one.to, hs);

		if (cmp_1 == 0 && cmp_2 == 0) {
			garlic.erase(it);
			break;
		}
	}

	this->mute.unlock();
}
/**
*	_garlic::find - Поиск нужного элемента в общем
*	списке статусов.
*
*	@one - Структура данных для записи результата.
*	@from - Хэш отправителя
*	@to - Хэщ получателя.
*/
bool _garlic::find(struct tgn_garlic &one,
	unsigned char *from, unsigned char *to)
{
	using tgnstruct::garlic;

	bool status = false;
	int cmp_1, cmp_2;

	if (!from || !to || garlic.size() == 0)
		return false;

	this->mute.lock();

	for (auto &p : garlic) {
		cmp_1 = memcmp(p.from, from, HASHSIZE);
		cmp_2 = memcmp(p.to, to, HASHSIZE);

		if (cmp_1 == 0 && cmp_2 == 0) {
			status = true;
			one = p;
			break;
		}
	}

	this->mute.unlock();
	return status;
}
/**
*	_garlic::exists - Проверка сеществования записи
*	в общем списке.
*
*	@one - Структура данных для поиска.
*/
bool _garlic::exists(struct tgn_garlic one)
{
	using tgnstruct::garlic;

	bool status = false;
	int cmp_1, cmp_2;

	if (garlic.size() == 0)
		return false;

	this->mute.lock();

	for (auto &p : garlic) {
		cmp_1 = memcmp(p.from, one.from, HASHSIZE);
		cmp_2 = memcmp(p.to, one.to, HASHSIZE);

		if (cmp_1 == 0 && cmp_2 == 0) {
			status = true;
			break;
		}
	}

	this->mute.unlock();
	return status;
}