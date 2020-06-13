
#include "../include/encryption.hpp"
#include "../include/storage.hpp"
#include "../include/network.hpp"
#include "../include/spruce.hpp"

void user_api(size_t port) {
	socklen_t sz = sizeof(struct sockaddr_in);
	struct sddr_structs srv, cln;
	unsigned char buff[PACKLEN], *ret;
	int usock, rs = -1;
	size_t len;

	usock = new_socket(SOCK_STREAM, TIMEOUT);
	assert(usock != 0 && port != 0);
	set_sockaddr(srv.sddr, port);

	if (bind(usock, srv.ptr, sz) != 0
		|| listen(usock, 5) != 0) {
		cout << "[W]: Can't start binding / listen"
			 << "ing." << endl;
		CLOSE_SOCKET(usock);
		return;
	}

	while (network.work) {
		if (rs < 0 && (rs = accept(usock, cln.ptr, &sz)) < 0) {
			continue;
		}

		memset(buff, 0x00, PACKLEN);

		len = recv(rs, buff, PACKLEN, 0);
		ret = userapi::processing(buff, len);

		if (len > 0) {
			if (send(rs, ret, len, 0) == -1) {
				rs = -1;
			}

			delete[] ret;
		}

		rs = (len == 0) ? -1 : rs;
	}
}

int main(void) {
	using structs::father;
	using structs::keys;
	using structs::role;
	using storage::db;

	string pub = db.get_var("PUBLIC_KEY");
	string sec = db.get_var("SECRET_KEY");
	string api = db.get_var("API_PORT");
	const short hs = HASHSIZE * 2;

	if (pub.length() == hs && sec.length() == hs) {
		keys.pub = hex2bin(hs, pub);
		keys.sec = hex2bin(hs, sec);
	}
	else {
		encryption::new_keys();

		pub = bin2hex(HASHSIZE, keys.pub);
		sec = bin2hex(HASHSIZE, keys.sec);

		db.set_var("PUBLIC_KEY", pub);
		db.set_var("SECRET_KEY", sec);
	}

	role = (db.get_var("UDP_USER") == "1") ? UDP_USER
										   : UDP_NONE;

	////////////////for removing!!!!!!!!!!!!!!!!!!!!!!!
	if (pub == "46e68bc9018936b1a2b466283341f9d48139420a78e683641e458459967e7b44") {
		role = UDP_NODE;
	}
	////////////////for removing!!!!!!!!!!!!!!!!!!!!!!!

	father.status = db.get_father(father.info.hash,
								  father.info.ipp.ip);
	father.info.ipp.port = UDP_PORT;
	father.info.ping = system_clock::now();

	storage::nodes.select();
	network.start();

	if (!network.work) {
		cout << "[E]: Can't start Network module.\n";
		return 1;
	}

	auto check_storage = [](void) {
		while (storage::check(), network.work);
	};

	thread check = thread(check_storage);

	user_api((api != "") ? stoi(api) : API_PORT);
	check.join();

	return 0;
}
/*
int main(void) {
	

	using structs::father;
	using structs::keys;
	using storage::db;

	string pub = db.get_var("PUBLIC_KEY");
	string sec = db.get_var("SECRET_KEY");
	const short hs = HASHSIZE * 2;

	cout << "nn: " << pub << endl;

	if (pub.length() == hs && sec.length() == hs) {
		keys.pub = hex2bin(hs, pub);
		keys.sec = hex2bin(hs, sec);

		cout << "pub: " << pub << endl;
	}
	else {
		encryption::new_keys();

		pub = bin2hex(HASHSIZE, keys.pub);
		sec = bin2hex(HASHSIZE, keys.sec);

		db.set_var("PUBLIC_KEY", pub);
		db.set_var("SECRET_KEY", sec);

		cout << "aa!!\n";
	}


	struct udp_task task;
	pack in, out;

	in.tmp(REQ_ROLE);
	task = in.to_task(structs::keys.pub);

	out = pack(task.buff, task.len);

	if (!out.is_correct()) {
		cout << "Not correct.\n";
	}
}*/