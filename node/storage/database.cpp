/**
*	database.cpp - Модуль отвечающий за работу
*	с базой данных в децентрализованной сети
*	TGN.
*
*	@mrrva - 2019
*/
#include "../include/storage.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_database::_database - Конструктор класса _database.
*	Заполняет переменные и указатели.
*/
_database::_database(void)
{
	string path = "./tgn.db";
	db = nullptr;

	if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
		cout << "[E] _database::_database.\n";
		exit(1);
	}

	this->create_tables();
}
/**
*	_database::_database - Диструктор класса _database.
*	Уничтожает переменные и указатели.
*/
_database::~_database(void)
{
	if (this->db != nullptr)
		sqlite3_close(this->db);
}
/**
*	_database::set_var - Добавление новых данных настроек
*	в базу данных.
*
*	@name - Имя параметра.
*	@value - Значение параметра.
*/
void _database::set_var(string name, string value)
{
	sqlite3_stmt *rs = nullptr;
	string q;

	if (name.length() < 2 || value.length() < 1) {
		cout << "[E] _database::set_var(args..).\n";
		return;
	}

	this->var_mutex.lock();

	q = "INSERT INTO settgngs VALUES('" + name + "', '" +
		value + "')";
	sqlite3_prepare_v2(this->db, q.c_str(), -1, &rs, NULL);

	if (sqlite3_step(rs) != SQLITE_DONE)
		cout << "[E] _database::set_var.\n";

	this->var_mutex.unlock();

	if (rs != nullptr)
		sqlite3_finalize(rs);
}
/**
*	_database::get_var - Извлечения данных настроек из
*	базы данных.
*
*	@name - Имя параметра.
*/
string _database::get_var(string name)
{
	sqlite3_stmt *rs = nullptr;
	typedef unsigned char uc;
	unsigned char *f_data;
	string q, r_data;

	if (name.length() < 2) {
		cout << "[E] _database::get_var(arg..).\n";
		return "";
	}

	this->var_mutex.lock();

	q = "SELECT `value` FROM `settgngs` WHERE `name`='" +
		name + "'";
	sqlite3_prepare_v2(this->db, q.c_str(), -1, &rs, NULL);

	if (sqlite3_step(rs) != SQLITE_ROW) {
		cout << "[E] _database::get_var.\n";

		if (rs != nullptr)
			sqlite3_finalize(rs);
		this->var_mutex.unlock();
		
		return "";
	}

	this->var_mutex.unlock();

	f_data = const_cast<uc *>(sqlite3_column_text(rs, 0));
	r_data = string(reinterpret_cast<char *>(f_data));

	if (rs != nullptr)
		sqlite3_finalize(rs);

	return r_data;
}
/**
*	_database::remove_var - Удаление данных настройки
*	из базы данных.
*
*	@name - Имя параметра.
*/
void _database::remove_var(string name)
{
	sqlite3_stmt *rs = nullptr;
	string q;

	if (name.length() < 2) {
		cout << "[E] _database::remove_var.\n";
		return;
	}

	this->var_mutex.lock();

	q = "DELETE FROM `settgngs` WHERE `name`='" +
		name + "'";

	sqlite3_prepare_v2(this->db, q.c_str(), -1, &rs, NULL);
	sqlite3_step(rs);

	this->var_mutex.unlock();

	if (rs != nullptr)
		sqlite3_finalize(rs);
}
/**
*	_database::remove_node - Удаление данных ноды
*	из базы данных.
*
*	@ip - Ip адрес ноды.
*/
void _database::remove_node(string ip)
{
	sqlite3_stmt *rs = nullptr;
	string q;

	if (ip.length() < 5) {
		cout << "[E] _database::remove_node.\n";
		return;
	}

	this->nodes_mutex.lock();

	q = "DELETE FROM `nodes` WHERE `ip`='" + ip + "'";
	sqlite3_prepare_v2(this->db, q.c_str(), -1, &rs, NULL);
	sqlite3_step(rs);

	this->nodes_mutex.unlock();

	if (rs != nullptr)
		sqlite3_finalize(rs);
}
/**
*	_database::new_node - Добавление новой ноды в базу
*	данных.
*
*	@ip - Ip адрес ноды.
*	@hash - Хэш ноды.
*/
void _database::new_node(string ip, string hash)
{
	sqlite3_stmt *rs = nullptr;
	time_t ping = time(NULL);
	stringstream ss;

	if (ip.length() < 5) {
		cout << "[E] _database::new_node.\n";
		return;
	}

	this->nodes_mutex.lock();

	ss << "INSERT INTO `nodes` VALUES (NULL, '" << ip
		<< "', '" << hash << "', '" << ping << "'');";
	sqlite3_prepare_v2(this->db, ss.str().c_str(), -1,
		&rs, NULL);
	sqlite3_step(rs);

	this->nodes_mutex.unlock();

	if (rs != nullptr)
		sqlite3_finalize(rs);
}
/**
*	_database::ping_node - Обновляем время последнего
*	использования ноды в базе данных.
*
*	@ip - Ip адрес ноды.
*/
void _database::ping_node(string ip)
{
	sqlite3_stmt *rs = nullptr;
	time_t ping = time(NULL);
	stringstream ss;

	if (ip.length() < 5) {
		cout << "[E] _database::ping_node.\n";
		return;
	}

	this->nodes_mutex.lock();

	ss << "UPDATE `nodes` SET `use_t`='" << ping
		<< "' WHERE `ip`='" << ip.c_str() << "'";
	sqlite3_prepare_v2(this->db, ss.str().c_str(),
		-1, &rs, NULL);
	sqlite3_step(rs);

	this->nodes_mutex.unlock();

	if (rs != nullptr)
		sqlite3_finalize(rs);
}
/**
*	_database::remove_old_nodes - Удаление всех старых нод
*	из базы данных.
*/
void _database::remove_old_nodes(void)
{
	time_t time_r = time(NULL) - 43200;
	sqlite3_stmt *rs = nullptr;
	stringstream ss;

	ss << "DELETE FROM `nodes` WHERE `use_t` < "
		<< time_r << ";";

	sqlite3_prepare_v2(this->db, ss.str().c_str(), -1,
		&rs, NULL);
	sqlite3_step(rs);

	if (rs != nullptr)
		sqlite3_finalize(rs);
}
/**
*	_database::select_nodes - Извлечение всех известных нод
*	из базы данных.
*/
map<string, string> _database::select_nodes(void)
{
	function<string(sqlite3_stmt *, int)> get_str;
	string q = "SELECT * FROM `nodes`", f, s;
	sqlite3_stmt *rs = nullptr;
	map<string, string> list;

	get_str = [](sqlite3_stmt *r, int n) {
		typedef unsigned char uc;
		unsigned char *data;
		char *buffer;

		data = const_cast<uc *>(sqlite3_column_text(r, n));
		buffer = reinterpret_cast<char *>(data);

		return string(buffer);
	};

	this->remove_old_nodes();
	sqlite3_prepare_v2(this->db, q.c_str(), -1, &rs, NULL);

	while (sqlite3_step(rs) == SQLITE_ROW) {
		f = get_str(rs, 1);
		s = get_str(rs, 2);
		list.insert(pair<string, string>(f, s));
	}

	if (rs != nullptr)
		sqlite3_finalize(rs);

	return list;
}
/**
*	_database::create_tables - Функция создания таблиц
*	базы данных сети.
*/
void _database::create_tables(void)
{
	vector<string> query = {
		"CREATE TABLE IF NOT EXISTS    " \
		"`nodes` (`id` INTEGER PRIMARY " \
		"KEY AUTOINCREMENT, `ip` text  " \
		"NOT NULL,`hash` text NOT NULL," \
		"`use_t` int(15) NOT NULL);",

		"CREATE TABLE IF NOT EXISTS    " \
		"`settgngs` (`name` text NOT   " \
		"NULL, `value` text NOT NULL);"
	};
	sqlite3_stmt *rs = nullptr;

	for (auto &str : query) {
		sqlite3_prepare_v2(this->db, str.c_str(), -1,
			&rs, NULL);

		if (sqlite3_step(rs) != SQLITE_DONE) {
			cout << "[E] _database::create_tables.\n";
			exit(1);
		}
	}

	if (rs != nullptr)
		sqlite3_finalize(rs);
}