
#include "../include/encryption.hpp"
#include "../include/network.hpp"
#include "../include/storage.hpp"
#include "../include/struct.hpp"

using namespace std;

using tinstruct::secret_key;
using tinstruct::public_key;
using tinstruct::neighbors;
using chrono::system_clock;
using tinstruct::nodes;
using chrono::time_point;
using tinstorage::db;

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
	struct tin_node node;
	struct tin_task task;
	unsigned char *buffer;
	string pub, sec;

	pub = db.get_var("PUBLIC_KEY");
	sec = db.get_var("SECRET_KEY");
	/**
	*	Getting secret and public keys.	
	*/
	if (pub.length() != hash_s || sec.length() != hash_s) {
		tinencryption::new_keys();

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
	*	Getting node list from db.	
	*/
	tinstorage::nodes.select();
	/**
	*	Starting threads.	
	*/
	if (!tinnetwork::socket.start()) {
		cout << "Can't start threads.\n";
		return 1;
	}
	/**
	*	Processing data from vectors.	
	*/
	while (true) {
		updnnbrs = tinstorage::neighbors.timelist();
		tinstorage::clients.remove();
		clock = system_clock::now();
// DNS REMOVING...
		/**
		*	Actions if neighbors list is empty.
		*/
		if (updnnbrs.empty()) {
			for (auto &p : nodes) {
				buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

				if (buffer == nullptr) {
					cout << "Error: Can't allocate mem.\n";
					return 1;
				}

				task.client_in = saddr_get(p.ip, PORT);
				memcpy(task.bytes, buffer, HEADERSIZE);
				task.length = HEADERSIZE;
				task.target_only = true;

				tinstorage::tasks.add(task);
				delete[] buffer;
			}
		}
		/**
		*	Updating exists list of neighbors.
		*/
		for (t = updnnbrs.begin(); t != updnnbrs.end(); t++) {
			if (clock - (*t).second < 900s)
				continue;

			buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

			if (buffer == nullptr) {
				cout << "Error: Can't allocate mem.\n";
				return 1;
			}

			if (!tinstorage::nodes.find_hash(node, (*t).first)) {
				tinstorage::neighbors.clear((*t).first);
				delete[] (*t).first;
				delete[] buffer;
				continue;
			}

			task.client_in = saddr_get(node.ip, PORT);
			memcpy(task.bytes, buffer, HEADERSIZE);
			task.length = HEADERSIZE;
			task.target_only = true;

			tinstorage::tasks.add(task);
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
				cout << "Error: Can't allocate mem.\n";
				return 1;
			}

			task.client_in = saddr_get(nodes[0].ip, PORT);
			memcpy(task.bytes, buffer, HEADERSIZE);
			task.target_only = false;
			task.length = HEADERSIZE;

			tinstorage::tasks.add(task);
			delete[] buffer;
			continue;
		}
		/**
		*	Removing all inactive nodes. 
		*/
		for (auto &p : nodes) {
			if (clock - p.ping > 43200s || p.remove)
				tinstorage::nodes.remove(p.ip);
		}
	}

	tinnetwork::recv.join();
	return 0;
}
