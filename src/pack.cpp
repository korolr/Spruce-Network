#include "../include/pack.hpp"
/*********************************************************/
pack::pack(unsigned char *buff, size_t size) {
	unsigned char *bytes = nullptr;
	int trash;

	correct = false;
	trash_size = 0;

	if (!buff || size < UDP_PACK) {
		return;
	}
	// Сообщение может иметь любой размер, главное
	// условие что бы сообщение было не меньше чем
	// UDP_PACK + crypto_box_SEALBYTES и финальное
	// количество байт было четным. Далее сообщение
	// отчищается от лишнего.
	trash = size - (UDP_PACK + crypto_box_SEALBYTES);
	trash = (trash <= 0) ? 0 : trash / 2;

	bytes = encryption::unpack(buff + trash, UDP_PACK);

	if (!bytes) {
		return;
	}

	memset(buffer, 0x00,  UDP_PACK);
	memcpy(buffer, bytes, UDP_PACK);

	delete[] bytes;
	correct = true;

	if ((*buffer >= NODE_END && *buffer != REQ_ROLE
		&& *buffer != RES_ROLE) || (*buffer >= USER_END
		&& *buffer < NODE_REQ_PING) || *buffer == 0x0) {
		correct = false;
	}
}

/*********************************************************/
void pack::tmp(enum udp_type type, size_t cookie) {
	unsigned char t = static_cast<unsigned char>(type);
	unsigned char *key = structs::keys.pub;

	if (type == INDEFINITE) {
		correct = false;
		return;
	}

	cookie = (cookie == 0) ? random_cookie() : cookie;
	correct = true;

	memset(buffer, 0x00, UDP_PACK);

	memcpy(buffer + 1 + HASHSIZE, &cookie, 4);
	memcpy(buffer + 1, key, HASHSIZE);
	memcpy(buffer, &t, 1);
}
/*********************************************************/
size_t pack::is_req(void) {
	assert(correct);

	if (*buffer > NODE_RES_NODE) {
		return 0;
	}

	return (*buffer % 2 == 0) ? 1 : 2;
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
void pack::set_info(unsigned char *info, size_t len) {
	assert(info && len > 0);
	memcpy(buffer + 5 + HASHSIZE, info, len);
}
/*********************************************************/
void pack::add_info(size_t sp, void *bytes, size_t len) {
	assert(bytes && len != 0);
	memcpy(buffer + 5 + HASHSIZE + sp, bytes, len);
}
/*********************************************************/
void pack::set_correct(void) {
	correct = true;
}
/*********************************************************/
unsigned char *pack::bytes(void) {
	assert(correct);
	return buffer;
}
/*********************************************************/
enum udp_type pack::type(void) {
	assert(correct);
	return static_cast<enum udp_type>(*buffer);
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
void pack::gentrash_nix(void) {
	ifstream fd("/dev/urandom", ios::binary);
	trash_size = (trash_size <= 100 && trash_size > 0) ? trash_size
													   : 100;
	assert(fd.good());

	fd.read(reinterpret_cast<char *>(right), trash_size);
	fd.read(reinterpret_cast<char *>(left),  trash_size);
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
	size_t dop = UDP_PACK + crypto_box_SEALBYTES;
	unsigned char *bytes;
	struct udp_task task;

	assert(correct && hash);
	srand(time(nullptr));
	dop += (trash_size = rand() % 100);

	task.type = static_cast<enum udp_type>(*buffer);
	memcpy(&task.cookie, buffer + HASHSIZE + 1, 4);

	bytes = encryption::pack(hash, buffer, UDP_PACK);
	assert(bytes);

	if (sddr != nullptr) {
		task.sddr = *sddr;
	}

#ifdef _WIN32
	this->gentrash_win();
#else 
	this->gentrash_nix();
#endif

	memcpy(task.buff + dop, right, trash_size);
	memcpy(task.buff, left, trash_size);
	task.len = dop + trash_size;

	dop = UDP_PACK + crypto_box_SEALBYTES;
	memcpy(task.buff + trash_size, bytes, dop);

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

#endif