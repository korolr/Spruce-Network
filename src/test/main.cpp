
#include "../include/encryption.hpp"
#include "../include/network.hpp"
#include "../include/storage.hpp"
#include "../include/struct.hpp"

using namespace std;

using tgnstruct::secret_key;
using tgnstruct::public_key;
using tgnstruct::neighbors;
using chrono::system_clock;
using tgnstruct::nodes;
using chrono::time_point;
using tgnstorage::db;

void ip_converter_test(string ip)
{
	unsigned char *b = iptobytes(ip);

	print_bytes<4>(b);
	cout << ipfrombytes(b) << "\n";

	exit(0);
}

int main(int argc, char *argv[])
{
	const size_t hash_s = HASHSIZE * 2;
	time_point<system_clock> clock;
	vector<time_list>::iterator t;
	vector<time_list> updnnbrs;
	struct tgn_node node;
	struct tgn_task task;
	unsigned char *buffer;
	string pub, sec;

	pub = db.get_var("PUBLIC_KEY");
	sec = db.get_var("SECRET_KEY");
	/**
	*	Gettgng secret and public keys.	
	*/
	if (pub.length() != hash_s || sec.length() != hash_s) {
		tgnencryption::new_keys();

		pub = bin2hex<HASHSIZE>(public_key);
		sec = bin2hex<HASHSIZE>(secret_key);

		db.set_var("PUBLIC_KEY", pub);
		db.set_var("SECRET_KEY", sec);
	}
	else {
		public_key = hex2bin<hash_s>(pub);
		secret_key = hex2bin<hash_s>(sec);
	}
	/**
	*	Gettgng node list from db.	
	*/
	tgnstorage::nodes.select();
	/**
	*	Startgng threads.	
	*/
	if (!tgnnetwork::socket.start()) {
		cout << "[E] tgnnetwork::socket.start.\n";
		return 1;
	}
	/**
	*	Processing data from vectors.	
	*/
	while (true) {
		updnnbrs = tgnstorage::neighbors.timelist();
		tgnstorage::clients.remove();
		tgnstorage::routes.remove();
		clock = system_clock::now();
		/**
		*	Actions if neighbors list is empty.
		*/
		if (updnnbrs.empty()) {
			for (auto &p : nodes) {
				buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

				if (buffer == nullptr) {
					cout << "[E] Allocationg of memory.\n";
					return 1;
				}

				task.client_in = saddr_get(p.ip, PORT);
				memcpy(task.bytes, buffer, HEADERSIZE);
				task.length = HEADERSIZE;
				task.target_only = true;

				tgnstorage::tasks.add(task);
				delete[] buffer;
			}
		}
		/**
		*	Updatgng exists list of neighbors.
		*/
		for (t = updnnbrs.begin(); t != updnnbrs.end(); t++) {
			if (clock - (*t).second < 900s)
				continue;

			buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

			if (buffer == nullptr) {
				cout << "[E] Allocationg of memory.\n";
				return 1;
			}

			if (!tgnstorage::nodes.find_hash(node, (*t).first)) {
				tgnstorage::neighbors.clear((*t).first);
				delete[] (*t).first;
				delete[] buffer;
				continue;
			}

			task.client_in = saddr_get(node.ip, PORT);
			memcpy(task.bytes, buffer, HEADERSIZE);
			task.length = HEADERSIZE;
			task.target_only = true;

			tgnstorage::tasks.add(task);
			delete[] (*t).first;
			delete[] buffer;
		}
		/**
		*	If there aren't any nodes in list. 
		*/
		if (nodes.size() <= MIN_NODES) {
			if (nodes.empty())
				continue;

			buffer = msg_tmp<true>(S_REQUEST_NODES);

			if (buffer == nullptr) {
				cout << "[E] Allocationg of memory.\n";
				return 1;
			}

			task.client_in = saddr_get(nodes[0].ip, PORT);
			memcpy(task.bytes, buffer, HEADERSIZE);
			task.target_only = false;
			task.length = HEADERSIZE;

			tgnstorage::tasks.add(task);
			delete[] buffer;
			continue;
		}
		/**
		*	Removing all inactive nodes. 
		*/
		for (auto &p : nodes) {
			if (clock - p.ping > 43200s || p.remove)
				tgnstorage::nodes.remove(p.ip);
		}
	}

	tgnnetwork::recv.join();
	return 0;
}
