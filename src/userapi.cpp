
#include "../include/userapi.hpp"

string userapi::processing(unsigned char *buff,
						   size_t len) {
	bool tobuff = false;
	string data;
	size_t slen;
	char c;

	if (len < 5 || len > PACKLEN || !buff) {
		return string();
	}

	args.clear();
	bsize = 0;

	for (size_t i = 0; i < len; i++) {
		slen = data.length();

		if (buff[i] == ' ' && slen > 0
			&& !tobuff) {
			args.push_back(data);
			continue;
		}

		if (buff[i] == '\n' && !tobuff) {
			if (slen > 0) {
				args.push_back(data);
			}
			tobuff = true;
			continue;
		}

		if (tobuff) {
			bytes[bsize] = buff[i];
			bsize++;
			continue;
		}

		memcpy(&c, buff + i, sizeof(char));
		data.push_back(c);
	}

	if (args[0] == "inbox") {
		return this->inbox();
	}

	if (args[0] == "newmsg") {
		return this->newmsg();
	}

	if (args[0] == "status") {
		return this->msgstatus();
	}

	return api_errs[0];
}



string userapi::msgstatus(void) {
	stringstream ss;
	size_t status;

	if (args.size() != 2) {
		return api_errs[1];
	}

	status = storage::msgs.info(stoi(args[1]));
	ss << "{\"error\" : false, \"status\" : ";
	ss << status;
	ss << "}";

	return ss.str();
}




string userapi::inbox(void) {
	map<string, string>::iterator it;
	map<string, string> list;
	string line;

	if (args.size() != 0) {
		return api_errs[2];
	}

	line = "{\"error\" : false, \"list\" : [";
	list = storage::inbox.list();

	for (it = list.begin(); it != list.end(); it++) {
		line += "{\"hash\" : \"" + it->first + "\", ";
		line += "\"file\" : \"" + it->second + "\"},";
	}

	line.pop_back();
	return (line += "]}");
}




string userapi::newmsg(void) {
	using storage::msgs;

	size_t size = args.size();
	string res = api_errs[9];
	unsigned char *hash;
	size_t hs = HASHSIZE * 2;

	if (size != 3 && size != 4) {
		return api_errs[3];
	}

	if (args[1].size() != hs) {
		return api_errs[4];
	}

	hash = hex2bin(hs, args[1]);
	assert(hash);

	switch (stoi(args[2])) {
	case 0: {
		if (bsize < 1) {
			res = api_errs[5];
			break;
		}

		msgs.add_bytes(hash, bytes, bsize);
		break;
	}

	case 1: {
		if (size != 4) {
			res = api_errs[6];
			break;
		}

		ofstream fd(args[3]);

		if (!fd.good()) {
			res = api_errs[7];
			break;
		}

		fd.close();
		msgs.add_file(hash, args[3]);
		break;
	}

	default:
		res = api_errs[8];
	}

	if (hash) { delete[] hash; }

	return res;
}

/*
sendmsg dsfgsdfgh454h54h file(1)/bytes(0)
*bytecoode*

{"error" : 0}



msgstatus *num*

{"status" : 0, "error" : 0}


inbox

[
	{"from" : "sfdg45g4g4gsdfg", "file" : "/tmp/file23f54g"},
	{"from" : "sfdg45g4g4gsdfg", "file" : "/tmp/file23f54g"},
	{"from" : "sfdg45g4g4gsdfg", "file" : "/tmp/file23f54g"},
	{"from" : "sfdg45g4g4gsdfg", "file" : "/tmp/file23f54g"}
]

{
	"error" : 0,
	"list" : [
		{"aa":"sdfsdf","bb":"bb"},
		{"aa":"sdfsdf","bb":"bb"},
		{"aa":"sdfsdf","bb":"bb"},
		{"aa":"sdfsdf","bb":"bb"}
	]
}
*/