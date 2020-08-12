
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

	assert(port != 0);

	usock = new_socket(SOCK_STREAM, TIMEOUT);
	set_sockaddr(srv.sddr, port);

	if (bind(usock, srv.ptr, sz) != 0
		|| listen(usock, 5) != 0) {
		cout << "[W]: Can't start binding / listen"
			 << "ing." << endl;
		CLOSE_SOCKET(usock);
		return;
	}

	while (network.work && structs::role != UDP_NODE) {
		if (rs < 0 && (rs = accept(usock, cln.ptr, &sz)) < 0) {
			continue;
		}

		memset(buff, 0x00, PACKLEN);

		len = recv(rs, buff, PACKLEN, 0);
		ret = spruceapi::processing(buff, len);

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
	constexpr short hs = HASHSIZE * 2;

	encryption::new_pair(&structs::fkeys);

	if (pub.length() == hs && sec.length() == hs) {
		keys.pub = hex2bin(hs, pub);
		keys.sec = hex2bin(hs, sec);
	}
	else {
		encryption::new_pair(&structs::keys);
		
		pub = bin2hex(HASHSIZE, keys.pub);
		sec = bin2hex(HASHSIZE, keys.sec);

		db.set_var("PUBLIC_KEY", pub);
		db.set_var("SECRET_KEY", sec);
	}

	role = (db.get_var("UDP_USER") == "1") ? UDP_USER
										   : UDP_NONE;
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

	if (role == UDP_USER) {
		user_api((api != "") ? stoi(api) : API_PORT);
	}

	if (check.joinable()) check.join();

	return 0;
}
