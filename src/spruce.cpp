
#include "../include/encryption.hpp"
#include "../include/storage.hpp"
#include "../include/network.hpp"
#include "../include/userapi.hpp"
#include "../include/spruce.hpp"

void user_api(size_t port) {
	socklen_t sz = sizeof(struct sockaddr_in);
	struct sddr_structs srv, cln;
	unsigned char buff[PACKLEN];
	int usock, rs;
	userapi api;
	size_t len;
	string tmp;

	usock = new_socket(SOCK_STREAM, TIMEOUT);
	assert(usock != 0 && port != 0);
	set_sockaddr(srv.sddr, port);

	if (bind(usock, srv.ptr, sz) != 0) {
		cout << "[W]: Incorrect API port.\n";
		CLOSE_SOCKET(usock);
		return;
	}

	if (listen(usock, 5) != 0) {
		cout << "[W]: Can't start listening.\n";
		CLOSE_SOCKET(usock);
		return;
	}

	while (network.work) {
		if ((rs = accept(usock, cln.ptr, &sz)) < 0) {
			continue;
		}

		memset(buff, 0x00, PACKLEN);

		len = recv(rs, buff, PACKLEN, 0);
		tmp = api.processing(buff, len);

		if ((len = tmp.length() + 1) < 5) {
			continue;
		}

		send(rs, tmp.c_str(), len, 0);
	}
}

int main(void) {
	using structs::father;
	using structs::keys;
	using structs::role;
	using storage::db;

	string pub = db.get_var("PUBLIC_KEY");
	string sec = db.get_var("SECRET_KEY");
	const short hs = HASHSIZE * 2;
	string apistr;
	thread check;

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

	check = thread(check_storage);

	apistr = db.get_var("API_PORT");
	user_api((apistr != "") ? stoi(apistr) : API_PORT);

	check.join();

	return 0;
}
