
#ifndef _SPRUCE_PACK_FATHER_
#define _SPRUCE_PACK_FATHER_

#include "../pack.hpp"

//[[ip][hash]...]
class res_father : public pack_tmp {
public:
	res_father(const unsigned char *b = nullptr)
		: pack_tmp(b) { type = RES_FATHER; }

	vector<struct haship>
	list(vector<struct haship> list = {}) {
		constexpr size_t ih = HASHSIZE + 4;
		constexpr size_t lim = INFOSIZE / ih;
		vector<struct haship> data;
		struct haship one;
		unsigned char *ip;
		size_t pos = 0;

		for (auto &p : list) {
			assert(ip = ip2bin(p.ip));
			HASHCPY(buffer + pos + 4, p.hash);
			memcpy(buffer + pos, ip, 4);
			delete[] ip;

			pos += HASHSIZE + 4;
		}

		if (!list.empty()) {
			return list;
		}

		for (size_t i = 0; i < lim; i++) {
			HASHCPY(one.hash, buffer + (i * ih) + 4);
			one.ip = bin2ip(buffer + (i * ih));

			if (IS_NULL(one.hash, HASHSIZE)) {
				break;
			}

			data.push_back(one);
		}

		return data;
	}
};

#endif