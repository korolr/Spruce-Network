
// find hash - status(1) - hash from - ip from
// 180s - время хранения маршрута без подтверждения, а то вдруг
// пользователь сменил отца, тогда - отправляем повторный запрос
// на подтверждение отцу, что клиент все еще с ним,
 // [HASH][STATUS][HASH][IP]
//
 // 1 ---[HASH]---> 2 ---[HASH][STATUS][HASH]--> 3 ---[HASH][STATUS][HASH][IP]---> 4
// 0x00 - В процессе
// 0x01 - Клиента нет
// 0x02 - Клиент найден

#include "../../include/network.hpp"
/*********************************************************/
struct ret find_router::req(struct sockaddr_in sddr,
							pack msg) {
	using structs::father;

	if (is_null(msg.info() + HASHSIZE + 1, HASHSIZE)) {
		return this->from_client(sddr, msg);
	}

	if (is_null(msg.info() + HASHSIZE * 2 + 1, 4)) {
		unsigned char *ip = ip2bin(ipport_get(sddr).ip);
		assert(ip);

		msg.add_info(HASHSIZE * 2 + 1, ip, 4);
		delete[] ip;
	}

	if ((IS_ME(msg.info()) || IS_FATHER(msg.info()))
		&& !storage::clients.exists(msg.info())) {
		pack req, res;

		req.tmp(NODE_REQ_FIND);
		req.set_info(msg.info(), HASHSIZE * 3);

		storage::tasks.add(req, sddr_get(father.info.ipp),
						   father.info.hash);

		res.tmp(NODE_RES_FIND, msg.cookie());
		res.set_info(msg.info(), HASHSIZE);

		return handler::make_ret(sddr, msg.hash(), res);
	}

	return this->from_chain(sddr, msg);
}
/*********************************************************/
bool find_router::allow_request(pack msg) {
	auto time = system_clock::now();
	struct find_req one;
	bool status = true;
	int cmp1, cmp2;

	mute.lock();

	for (size_t i = 0; i < reqs.size(); i++) {
		if (time - reqs[i].time > 30s)  {
			reqs.erase(reqs.begin() + i);
		}
	}

	for (auto &p : reqs) {
		cmp1 = memcmp(p.from, msg.hash(), HASHSIZE);
		cmp2 = memcmp(p.hash, msg.info(), HASHSIZE);

		if (cmp1 == 0 && cmp2 == 0) {
			status = false;
			break;
		}
	}

	if (status) {
		one.time = system_clock::now();
		HASHCPY(one.hash,  msg.info());
		HASHCPY(one.from,  msg.hash());
		reqs.push_back(one);
	}

	mute.unlock();
	return status;
}
/*********************************************************/
struct ret find_router::from_client(struct sockaddr_in sddr,
									pack msg) {
	using storage::father;
	using storage::tasks;
	using structs::keys;

	unsigned char *info = msg.info(), status;
	vector<struct route>::iterator it;
	auto time = system_clock::now();
	struct client node;
	pack res, req;

	res.tmp(NODE_RES_FIND, msg.cookie());
	res.set_info(info, HASHSIZE);

	bool ex1 = storage::routes.find(info, it);
	bool ex2 = storage::clients.exists(info);
	size_t s = storage::finds.check(info);

	// Если маршрут проходит по всем условиям 
	if ((ex1 && time - it->time <= 180s) || ex2 || s != 0) {
		status = s;
		res.add_info(HASHSIZE, &status, 1);
		return handler::make_ret(sddr, msg.hash(), res);
	}

	req.tmp(NODE_REQ_FIND);
	req.set_info(info, HASHSIZE);
	req.add_info(HASHSIZE + 1, keys.pub, HASHSIZE);
	// если маршрут давно не использовался - на подтверждение
	if (ex1) {
		tasks.add(req, sddr_get(it->ipp), it->father);
		return handler::make_ret(sddr, msg.hash(), res);
	}

	storage::finds.add(info);
	storage::routes.set(false, info, nullptr, {});

	if (IS_ME(info) || IS_FATHER(info)) {
		node = structs::father.info;
	}
	else {
		node = storage::clients.nearest(info);
		// Мы - лучше чем ближайшая нода. Если у нас клиента
		// нет - его нет в сети.
		if (father.cmp(info, keys.pub, node.hash) == 1) {
			status = 0x01;
			res.add_info(HASHSIZE, &status, 1);
			return handler::make_ret(sddr, msg.hash(), res);
		}
	}

	tasks.add(req, sddr_get(node.ipp), node.hash);
	return handler::make_ret(sddr, msg.hash(), res);
}
/*********************************************************/
struct ret find_router::from_chain(struct sockaddr_in sddr,
								   pack msg) {
	using storage::father;
	using storage::tasks;
	using structs::keys;

	unsigned char *info = msg.info(), status = 0x02;
	vector<struct route>::iterator it;
	auto time = system_clock::now();
	size_t shift = HASHSIZE + 1;
	struct client node;
	pack res1, res2;

	res1.tmp(NODE_RES_FIND, msg.cookie());
	res1.set_info(info, HASHSIZE);

	res2.tmp(NODE_REQ_FIND);
	res2.set_info(info, shift + HASHSIZE + 4);

	bool ex1 = storage::routes.find(info, it);
	bool ex2 = storage::clients.exists(info);

	if (ex1 && time - it->time <= 180s) {
		tasks.add(res2, sddr_get(it->ipp), it->hash);
		return handler::make_ret(sddr, msg.hash(), res1);
	}

	if (!this->allow_request(msg) && !ex2) {
		return handler::make_ret(sddr, msg.hash(), res1);
	}

	struct sockaddr_in fsddr = sddr_get({
		UDP_PORT,
		bin2ip(info + shift + HASHSIZE)
	});

	if (ex2) {
		res2.tmp(NODE_RES_FIND);
		res2.add_info(HASHSIZE, &status, 1);
		res2.set_info(info, HASHSIZE);

		tasks.add(res2, fsddr, info + shift);
		return handler::make_ret(sddr, msg.hash(), res1);
	}

	node = storage::clients.nearest(info);
	// Мы - лучше чем ближайшая нода. Если у нас клиента
	// нет - его нет в сети.
	if (father.cmp(info, keys.pub, node.hash) == 1) {
		status = 0x01;
		res2.tmp(NODE_RES_FIND);
		res2.add_info(HASHSIZE, &status, 1);
		res2.set_info(info, HASHSIZE);

		HASHCPY(node.hash, info + shift);
	}
	else {
		fsddr = sddr_get(node.ipp);
	}

	tasks.add(res2, fsddr, node.hash);
	return handler::make_ret(sddr, msg.hash(), res1);
}
/*********************************************************/
void find_router::res(struct sockaddr_in sddr, pack msg) {
	using storage::routes;

	unsigned char *info = msg.info(), status;
	vector<struct route>::iterator it;

	storage::tasks.rm_cookie(msg.cookie());
	status = *(info + HASHSIZE);

	if (status == 0x01) {
		storage::routes.rm_hash(info);
	}
	
	storage::finds.update(info, status);

	if (structs::role == UDP_USER) {
		return;
	}

	if (status == 0x02) {
		if (storage::routes.find(info, it)) {
			it->time = system_clock::now();
			return;
		}

		routes.update(info, msg.hash(), ipport_get(sddr));
	}
}


