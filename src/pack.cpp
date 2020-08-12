#include "../include/pack.hpp"
/*********************************************************/
pack::pack(unsigned char *buff, size_t size) {
	using encryption::unpack;

	constexpr size_t esize = UDP_PACK + crypto_box_SEALBYTES;
	unsigned char *bytes;

	if (!buff || size < UDP_PACK || size > esize + 200) {
		return;
	}

	size_t trash = size - esize;
	trash /= 2;

	bytes = unpack(buff + trash, UDP_PACK);

	if (bytes == nullptr) {
		return;
	}

	memset(buffer, 0x00,  UDP_PACK);
	memcpy(buffer, bytes, UDP_PACK);

	correct = VALID_TYPE(*bytes);
	delete[] bytes;
}
/*********************************************************/
void pack::set(const unsigned char *info, size_t len) {
	assert(info && len != 0);
	memcpy(buffer + 5 + HASHSIZE, info, len);
}
/*********************************************************/
void pack::tmp(enum udp_type type, size_t cookie) {
	using structs::role;

	size_t num = (cookie == 0) ? random_cookie()
							   : cookie;
	unsigned char *key = (role == UDP_NODE)
							   ? structs::keys.pub
							   : structs::fkeys.pub;

	memcpy(buffer + HASHSIZE + 1, &num, 4);
	HASHCPY(buffer + 1, key);
	memset(buffer, type, 1);
	correct = true;
}
/*********************************************************/
enum udp_type pack::type(void) {
	assert(correct);
	return static_cast<enum udp_type>(*buffer);
}
/*********************************************************/
enum udp_role pack::role(void) {
	assert(correct);

	if (*buffer == DOS || *buffer >= NODE_END) {
		return UDP_NONE;
	}

	if (*buffer < USER_END) {
		return UDP_USER;
	}

	if (*buffer > USER_END && *buffer < NODE_END) {
		return UDP_NODE;
	}

	return UDP_NONE;
}
/*********************************************************/
bool pack::is_correct(void) {
	return correct;
}
/*********************************************************/
unsigned char *pack::hash(void) {
	assert(correct);
	return buffer + 1;
}
/*********************************************************/
unsigned char *pack::info(void) {
	assert(correct);
	return buffer + HASHSIZE + 5;
}
/*********************************************************/
size_t pack::cookie(void) {
	size_t cookie;

	assert(correct);
	memcpy(&cookie, buffer + 1 + HASHSIZE, 4);

	return cookie;
}
/*********************************************************/
void pack::change_cookie(void) {
	size_t cookie = random_cookie();
	memcpy(buffer + 1 + HASHSIZE, &cookie, 4);
}
/*********************************************************/
void pack::gentrash_nix(void) {
	ifstream fd("/dev/urandom", ios::binary);

	assert(fd.good());

	fd.read(reinterpret_cast<char *>(right), tsize);
	fd.read(reinterpret_cast<char *>(left),  tsize);
	fd.close();
}
/*********************************************************/
void pack::gentrash_win(void) {

}
/*********************************************************/
string pack::hash_str(void) {
	assert(correct);
	return bin2hex(HASHSIZE, buffer + 1);
}
/*********************************************************/
struct udp_task pack::to_task(unsigned char *hash,
							  struct sockaddr_in *sddr) {
	size_t const dop = UDP_PACK + crypto_box_SEALBYTES;
	unsigned char *bytes;
	struct udp_task task;

	assert(correct);

	task.type = static_cast<enum udp_type>(*buffer);
	memcpy(&task.cookie, buffer +  HASHSIZE + 1, 4);

	srand(time(nullptr));

	tsize = rand() % 99;
	tsize = (tsize == 0) ? 99 : tsize;
	task.len = dop + tsize * 2;

	bytes = encryption::pack(hash, buffer, UDP_PACK);

	if (sddr != nullptr) {
		task.sddr = *sddr;
	}

#ifdef _WIN32
	gentrash_win();
#else 
	gentrash_nix();
#endif

	memcpy(task.buff + tsize + dop, right, tsize);
	memcpy(task.buff + tsize, bytes, dop);
	memcpy(task.buff, left, tsize);

	delete[] bytes;
	return task;
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void pack::debug(void) {
	string hex;

	if (!correct) {
		return;
	}

	hex = bin2hex(UDP_PACK, buffer);

	for (size_t i = 0; i < hex.length(); i++) {
		cout << hex[i];

		if (i % 2 != 0) {
			cout << " ";
		}
	}

	cout << endl << endl;
}

string pack::type_str(void) {
	assert(correct);

	switch(*buffer) {
	case REQ_FATHER: 		return "REQ_FATHER";
	case RES_FATHER: 		return "RES_FATHER";
	case REQ_ROLE: 			return "REQ_ROLE";
	case RES_ROLE: 			return "RES_ROLE";
	case USER_REQ_PING: 
	case NODE_REQ_PING: 	return "REQ_PING";
	case USER_RES_PING: 
	case NODE_RES_PING: 	return "RES_PING";
	case USER_REQ_TUNNEL: 
	case NODE_REQ_TUNNEL: 	return "REQ_TUNNEL";
	case USER_RES_TUNNEL: 
	case NODE_RES_TUNNEL: 	return "RES_TUNNEL";
	case USER_REQ_FIND: 
	case NODE_REQ_FIND: 	return "REQ_FIND";
	case USER_RES_FIND: 
	case NODE_RES_FIND: 	return "RES_FIND";
	case USER_REQ_PORT: 
	case NODE_REQ_PORT: 	return "REQ_PORT";
	case USER_RES_PORT: 
	case NODE_RES_PORT: 	return "RES_PORT";
	default: 				return "INDEFINITE";
	}
}

#endif