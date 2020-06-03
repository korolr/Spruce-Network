
#include "../include/spruce.hpp"
/**
*	Function to convert sockaddr_in structure to ipport structure
*/
struct ipport ipport_get(struct sockaddr_in a) {
	struct ipport ipp;
	char tmp[21];

	inet_ntop(AF_INET, &a.sin_addr, tmp, 20);
	ipp.port = ntohs(a.sin_port);
	ipp.ip = string(tmp);

	return ipp;
}
/**
*	Function to convert ipport structure to sockaddr_in structure
*/
struct sockaddr_in sddr_get(struct ipport ipp) {
	struct sockaddr_in sddr;

	if (ipp.ip.length() < 5) {
		return sddr;
	}

	sddr.sin_addr.s_addr = inet_addr(ipp.ip.c_str());
	sddr.sin_port = htons(ipp.port);

	return sddr;
}
/*
*	Random number generation.
*/
size_t random_cookie(void) {
	mt19937 gen; 
	gen.seed(time(0));

	return gen();
}
/*
*	Convert byte array to hex string.
*/
string bin2hex(size_t len, unsigned char *bytes) {
	char key[1000];

	assert(len < 1000);

	memset(key, 0x00, 1000);
	sodium_bin2hex(key, 1000, bytes, len);
	return string(key);
}
/*
*	Convert hex string to byte array
*/
unsigned char *hex2bin(size_t len, string line) {
	unsigned char *buff;
	size_t length;

	if (line.length() % 2 != 0) {
		return nullptr;
	}

	length = len / 2 + 1;
	buff = new unsigned char[length];
	assert(buff);

	sodium_hex2bin(buff, length, line.c_str(),
	               line.length(), NULL, NULL,
	               NULL);
	return buff;
}

bool is_null(unsigned char *buff, size_t len) {
	size_t res = 0;

	assert(buff && len > 0);

	for (size_t i = 0; i < len; i++) {
		res += static_cast<size_t>(buff[i]);
	}

	return res == 0;
}

unsigned char *ip2bin(string ip) {
	unsigned char *bytes, num;
	stringstream ss(ip);
	string buff;

	assert(bytes = new unsigned char[4]);

	for (size_t i = 0; i < 4; i++) {
		getline(ss, buff, '.');

		if ((num = stoi(buff)) > 254) {
			delete[] bytes;
			return nullptr;
		}

		bytes[i] = static_cast<unsigned char>(num);
	}

	return bytes;
}

string bin2ip(unsigned char *b) {
	size_t sum = 0;
	stringstream ss;

	assert(b);

	for (size_t i = 0; i < 4; i++) {
		if (b[i] > 254) {
			return string();
		}

		sum += static_cast<size_t>(b[i]);
	}

	if (sum == 0x00) {
		return string();
	}

	ss	<< static_cast<int>(b[0]) << "." 
		<< static_cast<int>(b[1]) << "." 
		<< static_cast<int>(b[2]) << "." 
		<< static_cast<int>(b[3]);

	return ss.str();
}

int new_socket(int type, size_t time) {
	int sol = SOL_SOCKET, opt, new_sock;
	struct timeval timeout;
	char *t_opt;

	if ((new_sock = socket(AF_INET, type, 0)) == 0) {
		return 0;
	}

	t_opt = reinterpret_cast<char *>(&timeout);
	timeout.tv_sec = time;
	opt = sizeof(timeout);
	timeout.tv_usec = 0;

	setsockopt(new_sock, sol, SO_RCVTIMEO, t_opt, opt);
	setsockopt(new_sock, sol, SO_SNDTIMEO, t_opt, opt);

	return new_sock;
}

void set_sockaddr(struct sockaddr_in &sddr, size_t port,
				  string ip) {
	sddr.sin_addr.s_addr = (ip.length() > 0) ? inet_addr(ip.c_str())
											 : INADDR_ANY;
	sddr.sin_port = htons(port);
	sddr.sin_family = AF_INET;
}