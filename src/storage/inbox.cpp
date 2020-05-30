
#include "../../include/storage.hpp"

void inbox_handler::add(unsigned char *hash, string file) {
	struct recvmsg one;

	assert(hash && file.length() > 0);

	one.hash = bin2hex(HASHSIZE, hash);
	one.id = this->free_id();
	one.file = file;

	mute.lock();
	structs::inbox.push_back(one);
	mute.unlock();
}

size_t inbox_handler::free_id(void) {
	vector<size_t>::iterator it;
	vector<size_t> ids;
	size_t free = 1;

	mute.lock();

	for (auto p : structs::inbox) {
		ids.push_back(p.id);
	}

	for (;; free++) {
		it = find(ids.begin(), ids.end(), free);
		assert(free != 0);

		if (it == ids.end()) {
			break;
		}
	}

	mute.unlock();
	return free;
}

void inbox_handler::rm_id(size_t id) {
	vector<struct recvmsg>::iterator it;

	mute.lock();
	it = structs::inbox.begin();

	for (; it != structs::inbox.end(); it++) {
		if ((*it).id != id) {
			continue;
		}

		remove((*it).file.c_str());
		structs::inbox.erase(it);
		break;
	}

	mute.unlock();
}

map<string, string> inbox_handler::list(void) {
	using structs::inbox;

	map<string, string> lmsgs;
	pair<string, string> one;
	size_t size;

	mute.lock();

	if ((size = inbox.size()) == 0) {
		mute.unlock();
		return lmsgs;
	}

	for (size_t i = 0; i < size; i++) {
		one = make_pair(inbox[i].hash, inbox[i].file);
		lmsgs.insert(one);

		inbox.erase(inbox.begin() + i);
	}

	mute.unlock();
	return lmsgs;
}