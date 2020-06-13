
#include "../../include/storage.hpp"
// 0 - not updated
// 1 - not found
// 2 - found
void finds_handler::add(unsigned char *hash) {
	bool res = false;
	struct find req;

	assert(hash);
	mute.lock();

	for (auto &p : structs::api::finds) {
		if (memcmp(p.hash, hash, HASHSIZE) != 0) {
			continue;
		}

		res = true;
		break;
	}

	if (res) {
		mute.unlock();
		return;
	}

	HASHCPY(req.hash, hash);
	req.time = system_clock::now();
	req.status = 0;

	structs::api::finds.push_back(req);
	mute.unlock();
}

size_t finds_handler::check(unsigned char *hash) {
	using structs::api::finds;

	vector<struct find>::iterator it;
	auto time = system_clock::now();
	size_t status = 0, i;

	assert(hash);
	mute.lock();

	for (i = 0; i < finds.size(); i++)   {
		if (time - finds[i].time > 30s)  {
			finds.erase(finds.begin() + i);
		}
	}

	for (i = 0; i < finds.size(); i++) {
		if (memcmp(finds[i].hash, hash, HASHSIZE) == 0) {
			status = finds[i].status;
			break;
		}
	}

	mute.unlock();
	return status;
}

void finds_handler::update(unsigned char *hash,
						   unsigned char  st) {
	size_t status = st;
	assert(hash);

	if (status == 0x00) {
		return;
	}
cout << "New status: " << status << endl;

	mute.lock();

	for (auto &p : structs::api::finds) {
		if (memcmp(p.hash, hash, HASHSIZE) == 0
			&& p.status <= status) {
			p.time = system_clock::now();
			p.status = status;
cout << "Updated!\n";
			break;
		}
	}

	mute.unlock();
}