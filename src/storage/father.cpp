
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
	storage::nodes.rm_hash(structs::father.info.hash);

	mute.lock();
	structs::father.status = false;
	mute.unlock();
}
/*********************************************************/
void father_handler::set(struct client new_father) {
	using structs::keys;

	if (IS_ME(new_father.hash)) {
		return;
	}

	mute.lock();

	new_father.ipp.port = UDP_PORT;
	new_father.ping = system_clock::now();
	structs::father.info = new_father;
	structs::father.status = true;

	mute.unlock();

	storage::db.set_father(structs::father);
}
/*********************************************************/
bool father_handler::from_father(struct sockaddr_in sddr,
								 pack msg) {
	using structs::father;

	struct ipport ipp = ipport_get(sddr);
	bool status = false;
	int cmp;

	if (!father.status) {
		return false;
	}

	mute.lock();
	cmp = memcmp(msg.hash(), father.info.hash, HASHSIZE);

	if (msg.type() > USER_END && msg.type() < 0x30
		&& cmp == 0 && father.info.ipp.ip == ipp.ip) {
		status = true;
	}

	mute.unlock();
	return status;
}
/*********************************************************/
void father_handler::check(void) {
	using structs::father;

	enum udp_type type;
	pack req;

	if (!structs::father.status) {
		storage::nodes.temporary_father();
		return;
	}

	if (system_clock::now() - ping <= 100s
		|| structs::role == UDP_NONE) {
		return;
	}

	type = (structs::role == UDP_USER) ? USER_REQ_FATHER
	                                   : NODE_REQ_FATHER;
	if (storage::tasks.exists(type)) {
		return;
	}

	ping = system_clock::now();
	req.tmp(type);
	storage::tasks.add(req, sddr_get(father.info.ipp),
	                   father.info.hash);
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