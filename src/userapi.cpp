
#include "../include/spruce.hpp"
#include "../include/storage.hpp"

unsigned char *userapi::processing(unsigned char *buff,
								   size_t &len) {
	unsigned char *res;

	if (!buff || len <= HASHSIZE) {
		len = 0;
		return nullptr;
	}

	if (structs::role == UDP_NONE) {
		len = 0;
		return nullptr;
	}

	switch (*buff) {
	case 0x00:
		res = userapi::find_req(buff, len);
		break;

	case 0x01:
		res = userapi::pack_req(buff, len);
		break;

	case 0x02:
		res = userapi::ibox_req(buff, len);
		break;

	default:
		res = nullptr;
		len = 0;
	}

	return res;
}

unsigned char *userapi::find_req(unsigned char *buff,
								 size_t &len) {
	using structs::father;
	using structs::role;

	unsigned char *res, c;
	pack msg;

	assert(res = new unsigned char[HASHSIZE + 1]);
	len = HASHSIZE + 1;

	msg.tmp((role == UDP_NODE) ? NODE_REQ_FIND
							   : USER_REQ_FIND);
	msg.set_info(buff + 1, HASHSIZE);

	storage::tasks.add(msg, sddr_get(father.info.ipp),
					   father.info.hash);
	storage::finds.add(buff + 1);

	size_t status = storage::finds.check(buff + 1);
	c = status;

	memcpy(res + HASHSIZE, &c, 1);
	HASHCPY(res, buff + 1);

	return res;
}

unsigned char *userapi::pack_req(unsigned char *buff,
								 size_t &len) {
	unsigned char *res;
	size_t port = 0;

	assert(res = new unsigned char[HASHSIZE + 4]);
	storage::msgs.add(buff + 1);
	len = HASHSIZE + 4;
	port = storage::msgs.create_thread(buff + 1);
cout << "Port: " << port << endl;

	memcpy(res + HASHSIZE, &port, 4);
	HASHCPY(res, buff + 1);

	return res;
}

unsigned char *userapi::ibox_req(unsigned char *buff,
								 size_t &len) {
	using structs::api::inbox;

	size_t size, shift = HASHSIZE + 4;
	unsigned char *res;

	storage::inbox.mute.lock();

	if ((size = inbox.size()) == 0) {
		storage::inbox.mute.unlock();
		len = 0;
		return nullptr;
	}

	len = (HASHSIZE + 4) * size;
	assert(res = new unsigned char[len]);

	for (size_t i = 0; i < size; i++) {
		HASHCPY(res + (i * shift), inbox[i].hash);
		memcpy(res + (i * shift) + HASHSIZE,
			   &inbox[i].port, 4);
	}

	inbox.clear();
	storage::inbox.mute.unlock();

	return res;
}