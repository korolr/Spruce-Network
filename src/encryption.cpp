
#include "../include/encryption.hpp"
/*********************************************************/
void encryption::new_keys(void) {
	assert(!structs::keys.pub && !structs::keys.sec);

	structs::keys.pub = new unsigned char[HASHSIZE];
	structs::keys.sec = new unsigned char[HASHSIZE];

	assert(structs::keys.pub && structs::keys.sec);
	crypto_box_keypair(structs::keys.pub, structs::keys.sec);
}
/*********************************************************/
unsigned char *encryption::pack(unsigned char *key,
	                            unsigned char *text,
	                            size_t size) {
	unsigned char *buff;
	size_t len;

	if (!key || !text || size < 1) {
		return nullptr;
	}

	len = size + 100 + crypto_box_SEALBYTES;
	buff = new unsigned char[len];

	assert(buff);

	memset(buff, 0x00, len);
	crypto_box_seal(buff, text, size, key);

	return buff;
}
/*********************************************************/
unsigned char *encryption::unpack(unsigned char *text,
	                              size_t size) {
	using structs::keys;

	unsigned char *buff = nullptr;
	size_t len;
	int s;

	len = size + crypto_box_SEALBYTES;
	buff = new unsigned char[len];

	assert(buff && text && size > crypto_box_SEALBYTES);

	memset(buff, 0x00, len);
	s = crypto_box_seal_open(buff, text, len, keys.pub,
                             keys.sec);

	if (s != 0) {
		delete[] buff;
		return nullptr;
	}

	return buff;
}