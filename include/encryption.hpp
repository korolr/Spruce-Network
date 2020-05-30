
#ifndef _SPRUCE_ENCRYPTION_
#define _SPRUCE_ENCRYPTION_

#include "spruce.hpp"

namespace encryption {
	unsigned char *pack(unsigned char *, unsigned char *, size_t);
	unsigned char *unpack(unsigned char *, size_t);
	void new_keys(void);
}

#endif