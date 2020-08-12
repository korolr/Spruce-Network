
#include "../include/spruce.hpp"
#include "../include/storage.hpp"
#include "../include/pack.hpp"

unsigned char *spruceapi::processing(unsigned char *buff,
									 size_t &size) {
	if (!buff || size == 0) {
		size = 0;
		return nullptr;
	}

	switch (*buff) {
	case 0x01:
		return spruce_status(size);

	case 0x02:
		return find_request(buff + 1, size);

	case 0x03:
		return tunnel_request(buff + 1, size);

	default:
		size = 0;
		return nullptr;
	}

	size = 0;
	return nullptr;
}

unsigned char *spruceapi::spruce_status(size_t &size) {
	unsigned char *buff = new unsigned char[HASHSIZE + 2];
	assert(buff);

	memset(buff + 1, ((structs::father.status) ? 1 : 0), 1);
	HASHCPY(buff + 2, structs::keys.pub);
	memset(buff, structs::role, 1);

	size = HASHSIZE + 2;
	return buff;
}

unsigned char *spruceapi::find_request(unsigned char *buff,
									   size_t &size) {
	using structs::father;

	unsigned char *ret;
	struct route fcln;
	req_find req;
	size_t code;
	pack msg;

	if (size < HASHSIZE || IS_NULL(buff, HASHSIZE)
		|| structs::role != UDP_USER || !father.status) {
		size = 0;
		return nullptr;
	}

	assert(ret = new unsigned char[HASHSIZE + 1]);
	HASHCPY(ret + 1, buff);
	size = HASHSIZE + 1;

	if (storage::routes.find(buff, fcln)) {
		memset(ret, fcln.status, 1);
		return ret;
	}

	code = random_cookie();

	req.father(storage::father.haship(), buff);
	req.from(structs::keys.pub, buff);
	req.code(code, buff);
	req.who(buff);

	msg.from(req);

	storage::routes.add(req.who(), code, PROGRESS);
	memset(ret, PROGRESS, 1);

	storage::tasks.add(msg, sddr_get(father.info.ipp),
					   father.info.hash);

	return ret;
}
// 0x00 - error (route doesn't exist), 0x02 - syncronization between nodes, 0x03 - good
unsigned char *spruceapi::tunnel_request(unsigned char *buff,
										 size_t &size) {
	using storage::routes;

	struct tcp_port port;
	unsigned char *ret;
	struct route fcln;

	if (size != HASHSIZE || IS_NULL(buff, HASHSIZE)) {
		size = 0;
		return nullptr;
	}

	assert(ret = new unsigned char[HASHSIZE + 5]);
	HASHCPY(ret + 1, buff);
	size = HASHSIZE + 5;
	*ret = 0x00;

	if (!routes.find(buff, fcln) || fcln.status != FOUND) {
		return ret;
	}

	/*if (if tunnel already exists!) {

	}*/


	return ret;
}