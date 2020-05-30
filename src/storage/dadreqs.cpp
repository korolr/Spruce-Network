
#include "../../include/storage.hpp"
/*********************************************************/
bool dadreqs_handler::exists(unsigned char *hash) {
	bool exists = false;

	assert(hash);

	mute.lock();

	for (auto &p : structs::dadreqs) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		exists = true;
		break;
	}

	mute.unlock();
	return exists;
}
/*********************************************************/
void dadreqs_handler::add(struct sockaddr_in sddr,
                          unsigned char *hash) {
	struct dadreq one;

	assert(hash);

	memcpy(one.hash, hash, HASHSIZE);
	one.time = system_clock::now();
	one.sddr = sddr;

	mute.lock();
	structs::dadreqs.push_back(one);
	mute.unlock();
}
/*********************************************************/
void dadreqs_handler::rm_hash(unsigned char *hash) {
	vector<struct dadreq>::iterator it;
	int cmp;

	assert(hash);

	mute.lock();
	it = structs::dadreqs.begin();

	for (; it != structs::dadreqs.end(); it++) {
		cmp = memcmp((*it).hash, hash, HASHSIZE);

		if (cmp != 0) {
			continue;
		}

		structs::dadreqs.erase(it);
		break;
	}

	mute.unlock();
}
/*********************************************************/
void dadreqs_handler::check(void) {
	using structs::dadreqs;

	time_point<system_clock> time;
	size_t size;

	mute.lock();
	time = system_clock::now();

	if ((size = dadreqs.size()) == 0) {
		mute.unlock();
		return;
	}

	for (size_t i = 0; i < size; i++) {
		if (time - dadreqs[i].time <= 600s) {
			continue;
		}

		dadreqs.erase(dadreqs.begin() + i);
	}

	mute.unlock();
}
/*********************************************************/
bool dadreqs_handler::find(struct dadreq &one,
                           unsigned char *hash) {
	bool status = false;

	assert(hash);
	mute.lock();

	for (auto &p : structs::dadreqs) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		status = true;
		one = p;
		break;
	}

	mute.unlock();
	return status;
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void dadreqs_handler::print(void) {
	struct ipport ipp;

	cout << "Hash, Ip" << endl
		 << "--------" << endl;

	mute.lock();

	for (auto &p : structs::dadreqs) {
		ipp = ipport_get(p.sddr);

		cout << bin2hex(HASHSIZE, p.hash)
			 << ", "  << ipp.ip << endl;
	}

	mute.unlock();
	cout << endl << endl;
}

#endif