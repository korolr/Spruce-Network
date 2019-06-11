
#ifndef TGN_NETWORK
#define TGN_NETWORK

#include "../include/network.hpp"

class _network {
	private :
		struct sockaddr_arr s_data;
		struct timeval timeout;

	public :
		~_network(void);
		_network(void);
};

#endif