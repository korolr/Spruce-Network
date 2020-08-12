
#include "../../include/storage.hpp"


/*********************************************************/
size_t father_handler::cmp(unsigned char *main,
						   unsigned char *h1,
						   unsigned char *h2) {
	int cmp1, cmp2;

	assert(h1 && h2 && main);

	for (size_t i = 0; i < HASHSIZE; i++) {
		cmp1 = abs(main[i] - h1[i]);
		cmp2 = abs(main[i] - h2[i]);

		if (cmp1 < cmp2) {
			return 1;
		}

		if (cmp1 > cmp2) {
			return 2;
		}
	}

	return 0;
}
/*********************************************************/
void father_handler::no_father(void) {
	mute.lock();
	storage::nodes.rm_hash(structs::father.info.hash);
	mute.unlock();

	structs::father.status = false;
}
/*********************************************************/
void father_handler::set(struct client nfather) {
	using structs::father;

	unsigned char *key = structs::keys.pub;

	mute.lock();

	if (IS_ME(nfather.hash) || (structs::father.status &&
		cmp(key, nfather.hash, father.info.hash) != 1)) {
		mute.unlock();
		return;
	}

	nfather.ipp.port = UDP_PORT;
	nfather.ping = system_clock::now();

	structs::father.info = nfather;
	structs::father.status = true;

	mute.unlock();
}
/*********************************************************/
bool father_handler::from_father(struct sockaddr_in sddr,
								 pack msg) {
	using structs::father;

	struct ipport ipp = ipport_get(sddr);
	bool status = false, eq;

	if (!father.status) {
		return false;
	}

	mute.lock();
	eq = HASHEQ(msg.hash(), father.info.hash);

	if (msg.type() > USER_END && msg.type() < 0x30
		&& eq && father.info.ipp.ip == ipp.ip) {
		status = true;
	}

	mute.unlock();
	return status;
}
/*********************************************************/
struct haship father_handler::haship(void) {
	using structs::father;

	struct haship data;

	mute.lock();
	HASHCPY(data.hash, father.info.hash);
	data.ip = (father.status) ? father.info.ipp.ip : string();
	mute.unlock();

	return data;
}
/*********************************************************/
void father_handler::check(void) {
	using structs::father;
	using storage::nodes;

	unsigned char *pub = structs::keys.pub;
	auto time = system_clock::now();
	bool cond;

	cond = (father.status && time - ping < 200s)
		|| (!father.status && time - ping < 3s)
		|| storage::tasks.exists(REQ_FATHER);

	if (cond) {
		return;
	}

	struct client node = storage::nodes.nearest(pub);
	unsigned char *fhash = father.info.hash;
	pack msg;

	mute.lock();

	if (father.status && cmp(pub, fhash, node.hash) == 2) {
		storage::nodes.rm_hash(fhash);
		father.status = false;
	}

	ping = system_clock::now();
	mute.unlock();

	msg.tmp(REQ_FATHER);
	storage::tasks.add(msg, sddr_get(node.ipp), node.hash);
}
/*********************************************************/
#if defined(DEBUG) && DEBUG == true

void father_handler::print(void) {
	using structs::father;

	if (!father.status) {
		cout << "No father.\n";
		return;
	}

	cout << "Hash, Ip, Active" << endl
		 << "----------------" << endl
		 << bin2hex(HASHSIZE, father.info.hash)
		 << ", " << father.info.ipp.ip << ", "
		 << ((father.status) ? "True" : "False")
		 << endl << endl;
}

#endif