
#include "../../include/network.hpp"



struct ret father_router::request(struct sockaddr_in sddr,
								  pack msg) {
	constexpr size_t lim = INFOSIZE / (HASHSIZE + 4);
	vector<struct haship> get, wrt;
	res_father fres;
	pack res;

	get = storage::clients.veclist();
	fres.cookie = msg.cookie();
	res.from(fres);

	if (structs::father.status) {
		get.push_back(storage::father.haship());
	}

	if (get.empty()) {
		return handler::make_ret(sddr, msg.hash(), res);
	}

	auto newpack = [&](struct sockaddr_in &sddr,
					   res_father &r, pack &c) {
		pack msg;

		msg.from(r);
		r.clear();

		storage::tasks.add(msg, sddr, c.hash());
	};

	while (get.size() > lim) {
		wrt = get;
		wrt.erase(wrt.begin() + lim,   wrt.end());
		get.erase(get.begin(), get.begin() + lim);
		fres.list(wrt);

		newpack(sddr, fres, msg);
	}

	fres.list(get);
	newpack(sddr, fres, msg);

	return handler::make_ret(sddr, msg.hash(), res);
}



void father_router::response(struct sockaddr_in sddr,
							 pack msg) {
	using structs::keys;

	struct client node = storage::nodes.nearest(keys.pub);
	struct ipport ipp = ipport_get(sddr);
	vector<struct haship> list;
	bool update = true;
	res_father res;

	storage::tasks.rm_cookie(msg.cookie());
	storage::nodes.add(msg.hash(), ipp.ip);

	mute.lock();

	if (dad.status && HASHEQ(dad.info.hash,
							 msg.hash())) {
		update = false;
		attempts++;
	}

	if (update) {
		HASHCPY(dad.info.hash, msg.hash());
		dad.info.ipp = ipp;
		dad.status = true;
		attempts = 0;
	}

	if (attempts >= 3 && HASHEQ(node.hash,
								msg.hash())) {
		storage::father.set(dad.info);
		dad.status = false;
		attempts = 0;
	}

	mute.unlock();

	msg.to(res);
	list = res.list();

	for (auto &p : list) {
		storage::nodes.add(p.hash, p.ip);
	}
}
