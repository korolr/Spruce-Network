
#include "../include/encryption.hpp"
/*********************************************************/
void encryption::new_pair(struct keypair *pair) {
	assert(!pair->pub && !pair->sec);

	pair->pub = new unsigned char[HASHSIZE];
	pair->sec = new unsigned char[HASHSIZE];

	assert(pair->pub && pair->sec);
	crypto_box_keypair(pair->pub, pair->sec);
}
/*********************************************************/
unsigned char *encryption::pack(unsigned char *key,
	                            unsigned char *text,
	                            size_t size) {
	unsigned char *buff;
	size_t len;

	assert(key && text && size > 0);

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
	using structs::fkeys;
	using structs::keys;

	unsigned char *buff = nullptr;
	size_t len;

	len = size + crypto_box_SEALBYTES;
	buff = new unsigned char[len];

	assert(buff && text && len > crypto_box_SEALBYTES);
	memset(buff, 0x00, len);

	if (crypto_box_seal_open(buff, text, len,
							 keys.pub,
							 keys.sec) != 0) {
		if (structs::role == UDP_NODE) {
			delete[] buff;
			return nullptr;
		}
	}
	else {
		return buff;
	}

	if (crypto_box_seal_open(buff, text, len,
							 fkeys.pub,
							 fkeys.sec) != 0) {
		delete[] buff;
		buff = nullptr;
	}

	return buff;
}