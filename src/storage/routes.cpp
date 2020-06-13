#include "../../include/storage.hpp"
/*********************************************************/
void routes_handler::set(bool full, unsigned char *hash,
						 unsigned char *father,
						 struct ipport ipp) {
	bool exists = false;
	struct route one;

	assert(hash);
	mute.lock();

	for (auto &p : structs::routes) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		exists = true;

		if (full) {
			if (p.full) {
				break;
			}

			p.time = system_clock::now();
			HASHCPY(one.father, father);
			p.full = true;
			p.ipp = ipp;
			break;
		}
	}

	if (exists) {
		mute.unlock();
		return;
	}

	one.time = system_clock::now();
	HASHCPY(one.hash, hash);
	one.full = full;
	one.ipp = ipp;

	if (full) {
		assert(father);
		HASHCPY(one.father, father);
	}

	structs::routes.push_back(one);
	mute.unlock();
}
/*********************************************************/
void routes_handler::update(unsigned char *hash,
							unsigned char *father,
							struct ipport ipp) {
	vector<struct route>::iterator it;
	bool found = false;

	assert(hash && father);

	mute.lock();
	it = structs::routes.begin();

	for (; it != structs::routes.end(); it++) {
		if (memcmp((*it).hash, hash, HASHSIZE) != 0) {
			continue;
		}

		if ((*it).full) {
			break;
		}

		found = true;
		break;
	}

	if (!found) {
		mute.unlock();
		return;
	}

	(*it).time = system_clock::now();
	HASHCPY((*it).father, father);
	(*it).full = true;
	(*it).ipp = ipp;

	mute.unlock();
}
/*********************************************************/
/*bool routes_handler::find(unsigned char *hash,
						  struct route &one) {
	bool found = false;

	assert(hash);
	mute.lock();

	for (auto &p : structs::routes) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		p.time = system_clock::now();
		found = true;
		one = p;
		break;
	}

	mute.unlock();
	return found;
}*/
bool routes_handler::exists(unsigned char *hash) {
	bool found = false;

	mute.lock();

	for (auto &p : structs::routes) {
		if (memcmp(p.hash, hash, HASHSIZE) == 0
			&& p.full) {
			found = true;
			break;
		}
	}

	mute.unlock();
	return found;
}
/*********************************************************/
bool routes_handler::find(unsigned char *hash,
						  vector<struct route>::iterator &it) {
	vector<struct route>::iterator i;
	bool found = false;

	assert(hash);

	i = structs::routes.begin();
	mute.lock();

	for (; i != structs::routes.end(); i++) {
		if (memcmp((*i).hash, hash, HASHSIZE) != 0
			|| !(*i).full) {
			continue;
		}

		found = true;
		it = i;
		break;
	}

	mute.unlock();
	return found;
}
/*********************************************************/
void routes_handler::rm_hash(unsigned char *hash) {
	vector<struct route>::iterator it;
	int cmp;

	assert(hash);
	mute.lock();

	it = structs::routes.begin();

	for (; it != structs::routes.end(); it++) {
		cmp = memcmp((*it).hash, hash, HASHSIZE);

		if (cmp != 0 || !(*it).full) {
			continue;
		}

		structs::routes.erase(it);
		break;
	}

	mute.unlock();
}
/*********************************************************/
void routes_handler::check(void) {
	using structs::routes;

	auto ctime = system_clock::now();
	size_t size;

	mute.lock();

	if ((size = routes.size()) == 0) {
		mute.unlock();
		return;
	}

	for (size_t i = 0; i < size; i++) {
		if (ctime - routes[i].time <= 1800s) {
			continue;
		}

		routes.erase(routes.begin() + i);
	}

	mute.unlock();
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void routes_handler::print(void) {
	struct ipport ipp;

	cout << "father(hash:ip), Client hash, Is full" << endl
		 << "-------------------------------------" << endl;

	mute.lock();

	for (auto &p : structs::routes) {
		cout << bin2hex(HASHSIZE, p.father) << ":"
			 << p.ipp.ip << ", "
			 << bin2hex(HASHSIZE, p.hash) << ", "
			 << ((p.full) ? "True" : "False") << endl;
	}

	mute.unlock();
	cout << endl << endl;
}

#endif