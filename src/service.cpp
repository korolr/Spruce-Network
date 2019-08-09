/**
*	service.cpp - Точка входа в программу.
*
*	@mrrva - 2019
*/
#include "include/encryption.hpp"
#include "include/network.hpp"
#include "include/storage.hpp"
#include "include/struct.hpp"

int main()
{
	using tgnstruct::secret_key;
	using tgnstruct::public_key;
	using tgnstorage::db;

	std::string pub = db.get_var("PUBLIC_KEY");
	std::string sec = db.get_var("SECRET_KEY");
	const short hs = HASHSIZE * 2;

	if (pub.length() == hs && sec.length() == hs) {
		public_key = hex2bin<hs>(pub);
		secret_key = hex2bin<hs>(sec);
	}
	else {
		tgnencryption::new_keys();

		pub = bin2hex<HASHSIZE>(public_key);
		sec = bin2hex<HASHSIZE>(secret_key);

		db.set_var("PUBLIC_KEY", pub);
		db.set_var("SECRET_KEY", sec);
	}

	tgnstorage::nodes.select();

	if (tgnstruct::nodes.size() == 0) {
		std::cout << "[E] Node list is empty. Please "
			<< "download database with current list "
			<< "of nodes.\n";
		return 1;
	}

	if (!tgnnetwork::socket.start()) {
		std::cout << "[E] socket.start.\n";
		return 1;
	}

	while (true) {
		tgnstorage::neighbors.autocheck();
		tgnstorage::clients.autoremove();
		tgnstorage::garlic.autoremove();
		tgnstorage::nodes.autocheck();
		tgnstorage::routes.autoremove();
	}

	tgnnetwork::recv.join();
	return 0;
}
