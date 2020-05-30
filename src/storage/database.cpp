
#include "../../include/storage.hpp"
/*********************************************************/
database_handler::database_handler(string path) {
	db = nullptr;

	if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
		cout << "[E]: Can't open database.\n";
		exit(1);
	}

	this->create_tables();
}
/*********************************************************/
database_handler::~database_handler(void) {
	if (db != nullptr) {
		sqlite3_close(db);
	}
}
/*********************************************************/
void database_handler::create_tables(void) {
	vector<string> query = {
		"CREATE TABLE IF NOT EXISTS    " \
		"`nodes` (`ip` text  NOT NULL, " \
		"`hash` text NOT NULL);",

		"CREATE TABLE IF NOT EXISTS    " \
		"`father` (`ip`  text NOT NULL," \
		"`hash` text NOT NULL);",

		"CREATE TABLE IF NOT EXISTS    " \
		"`settings` (`name` text NOT   " \
		"NULL, `value` text NOT NULL);"
	};
	sqlite3_stmt *rs = nullptr;

	for (auto &str : query) {
		sqlite3_prepare_v2(db, str.c_str(), -1,
		                   &rs, nullptr);

		assert(sqlite3_step(rs) == SQLITE_DONE);
	}

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}
}
/*********************************************************/
void database_handler::set_father(struct udp_father node) {
	size_t port = node.info.ipp.port;
	string ip = node.info.ipp.ip;
	sqlite3_stmt *rs = nullptr;
	string q, hash;

	if (ip.length() < 6 || port != UDP_PORT) {
		return;
	}

	mute.lock();

	q = "DELETE FROM `father`;";
	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);
	sqlite3_step(rs);

	if (rs != nullptr) {
		sqlite3_finalize(rs);
		rs = nullptr;
	}

	hash = bin2hex(HASHSIZE, node.info.hash);
	q = "INSERT INTO `father` VALUES('" + ip + "', '" +
		hash + "');";

	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);
	sqlite3_step(rs);

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}
}
/*********************************************************/
map<unsigned char *, string> database_handler::nodes(void) {
	string q = "SELECT * FROM `nodes`";
	map<unsigned char *, string> list;
	sqlite3_stmt *rs = nullptr;
	unsigned char *tmp;
	string ip, hash;

	auto to_str = [](const unsigned char *data) {
		unsigned char *tmp;

		assert(data);

		tmp = const_cast<unsigned char *>(data);
		return string(reinterpret_cast<char *>(tmp));
	};

	mute.lock();

	sqlite3_prepare_v2(db, q.c_str(), -1, &rs,
					   nullptr);

	while (sqlite3_step(rs) == SQLITE_ROW) {
		hash = to_str(sqlite3_column_text(rs, 1));
		tmp	= hex2bin(HASHSIZE * 2, hash);

		ip = to_str(sqlite3_column_text(rs, 0));

		if (tmp) {
			list.insert(make_pair(tmp, ip));
		}
	}

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}

	return list;
}
/*********************************************************/
void database_handler::rm_node(string ip) {
	sqlite3_stmt *rs = nullptr;
	string q;

	if (ip.length() < 6) {
		return;
	}

	mute.lock();

	q = "DELETE FROM `nodes` WHERE `ip`='" + ip + "';";
	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);
	sqlite3_step(rs);

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}
}
/*********************************************************/
void database_handler::set_var(string name, string value) {
	sqlite3_stmt *rs = nullptr;
	string q;

	if (name.length() < 2 || value.length() < 1) {
		return;
	}

	this->rm_var(name);
	mute.lock();

	q = "INSERT INTO `settings` VALUES('" + name + "', '"
		+ value + "')";
	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);

	if (sqlite3_step(rs) != SQLITE_DONE) {
		cout << "[E] _database::set_var.\n";
	}

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}
}
/*********************************************************/
void database_handler::rm_var(string name) {
	sqlite3_stmt *rs = nullptr;
	string q;

	if (name.length() < 2) {
		return;
	}

	mute.lock();

	q = "DELETE FROM `settings` WHERE `name`='" +
		name + "'";

	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);
	sqlite3_step(rs);

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}
}
/*********************************************************/
void database_handler::add_node(unsigned char *hash,
								string ip) {
	sqlite3_stmt *rs = nullptr;
	string q;

	if (!hash || ip.length() < 6) {
		return;
	}

	q = "INSERT INTO `nodes` VALUES ('" + ip + "', '"
		+ bin2hex(HASHSIZE, hash) + "');";

	mute.lock();

	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);
	sqlite3_step(rs);

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}
}


string database_handler::get_var(string name)
{
	sqlite3_stmt *rs = nullptr;
	typedef unsigned char uc;
	unsigned char *fdata;
	string q, rdata;

	if (name.length() < 2) {
		return "";
	}

	mute.lock();

	q = "SELECT `value` FROM `settgngs` WHERE `name`='" +
		name + "'";
	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);

	if (sqlite3_step(rs) != SQLITE_ROW) {
		if (rs != nullptr) {
			sqlite3_finalize(rs);
		}

		mute.unlock();
		return "";
	}

	fdata = const_cast<uc *>(sqlite3_column_text(rs, 0));
	rdata = string(reinterpret_cast<char *>(fdata));

	mute.unlock();

	if (rs != nullptr) {
		sqlite3_finalize(rs);
	}

	return rdata;
}

bool database_handler::get_father(unsigned char *hash,
								  string &ip) {
	string q = "SELECT * FROM `father` LIMIT 1";
	sqlite3_stmt *rs = nullptr;
	typedef unsigned char uc;
	unsigned char *tmp;
	string strhash;

	assert(hash);
	mute.lock();

	sqlite3_prepare_v2(db, q.c_str(), -1, &rs, nullptr);
	if (sqlite3_step(rs) != SQLITE_ROW) {
		if (rs != nullptr) {
			sqlite3_finalize(rs);
		}

		mute.unlock();
		return false;
	}

	tmp = const_cast<uc *>(sqlite3_column_text(rs, 1));
	strhash = string(reinterpret_cast<char *>(tmp));

	tmp = const_cast<uc *>(sqlite3_column_text(rs, 0));
	ip = string(reinterpret_cast<char *>(tmp));

	mute.unlock();

	assert(tmp = hex2bin(HASHSIZE * 2, strhash));
	memcpy(hash, tmp, HASHSIZE);
	delete[] tmp;

	return ip.length() > 6;
}
