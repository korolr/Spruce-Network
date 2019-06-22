/*
using namespace std;

void _service::select_keys(void)
{
	using tgnstruct::secret_key;
	using tgnstruct::public_key;
	using tgnstorage::db;

	string pub = db.get_var("PUBLIC_KEY");
	string sec = db.get_var("SECRET_KEY");
	const short hs = HASHSIZE * 2;

	if (pub.length() == hs && sec.length() == hs) {
		public_key = hex2bin<hash_s>(pub);
		secret_key = hex2bin<hash_s>(sec);
		return;
	}

	tgnencryption::new_keys();

	pub = bin2hex<HASHSIZE>(public_key);
	sec = bin2hex<HASHSIZE>(secret_key);

	db.set_var("PUBLIC_KEY", pub);
	db.set_var("SECRET_KEY", sec);
}

bool _service::start_threads(void)
{
	tgnstorage::nodes.select();

	if (tgnstruct::nodes.empty()) {
		cout << "[E] Node list is empty.\n"
		return false;
	}

	if (!tgnnetwork::socket.start()) {
		cout << "[E] Threads stoped.\n";
		return false;
	}

	return true;
}

void _service::neighbors_get(void)
{
	using tgnstorage::tasks;

	unsigned char *buffer;
	struct tgn_task task;

	for (auto &p : tgnstruct::nodes) {
		buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

		if (buffer == nullptr) {
			cout << "[E] Allocationg of memory.\n";
			exit(1);
		}

		task.client_in = saddr_get(p.ip, PORT);
		memcpy(task.bytes, buffer, HEADERSIZE);
		task.length = HEADERSIZE;
		task.target_only = true;

		tasks.add(task);
		delete[] buffer;
	}
}

void _service::neighbors(void)
{
	using tgnstorage::neighbors;
	using tgnstorage::nodes;
	using tgnstorage::tasks;

	vector<time_list> list = neighbors.timelist();
	vector<time_list>::iterator it;
	time_point<system_clock> clock;
	unsigned char *buffer;
	struct tgn_node node;
	struct tgn_task task;

	if (list.empty()) {
		this->neighbors_get();
		return;
	}

	clock = system_clock::now();

	for (it = list.begin(); it != list.end(); it++) {
		if (clock - (*it).second < 900s)
			continue;

		buffer = msg_tmp<true>(S_REQUEST_CLIENTS);

		if (buffer == nullptr) {
			cout << "[E] Allocationg of memory.\n";
			exit(1);
		}

		if (!nodes.find_hash(node, (*it).first)) {
			neighbors.clear((*it).first);
			delete[] (*it).first;
			delete[] buffer;
			continue;
		}

		task.client_in = saddr_get(node.ip, PORT);
		memcpy(task.bytes, buffer, HEADERSIZE);
		task.length = HEADERSIZE;
		task.target_only = true;

		tasks.add(task);
		delete[] (*it).first;
		delete[] buffer;
	}
}

void _service::nodes(void)
{
	using tgnstruct::nodes;
	using tgnstorage::tasks;

	time_point<system_clock> clock;
	unsigned char *buffer;
	struct tgn_task task;

	if (nodes.size() == 0) {
		cout << "[E] Node list is empty.\n"
		exit(1);
	}

	clock = system_clock::now();

	if (nodes.size() <= MIN_NODES) {
		buffer = msg_tmp<true>(S_REQUEST_NODES);

		if (buffer == nullptr) {
			cout << "[E] Allocationg of memory.\n";
			exit(1);
		}

		task.client_in = saddr_get(nodes[0].ip, PORT);
		memcpy(task.bytes, buffer, HEADERSIZE);
		task.target_only = false;
		task.length = HEADERSIZE;

		tasks.add(task);
		delete[] buffer;
		return;
	}

	for (auto &p : nodes) {
		if (clock - p.ping > 43200s || p.remove)
			tgnstorage::nodes.remove(p.ip);
	}
}

void _service::garlic(void)
{

}
*/