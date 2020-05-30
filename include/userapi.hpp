
#ifndef _SPRUCE_USERAPI_
#define _SPRUCE_USERAPI_

#include "spruce.hpp"
#include "storage.hpp"

inline const vector<string> api_errs = {
	"{\"error\" : true, \"msg\" : \"Unknown request.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect args for status.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect args for inbox.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect args for newmsg.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect hash for newmsg.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect byte array for newmsg.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect file path for newmsg.\"}",
	"{\"error\" : true, \"msg\" : \"Can't open file for sending.\"}",
	"{\"error\" : true, \"msg\" : \"Incorrect message type for newmsg.\"}",
	"{\"error\" : false}"
};

class userapi {
	private:
		unsigned char bytes[PACKLEN];
		vector<string> args;
		size_t bsize;

		string msgstatus(void);
		string inbox(void);
		string newmsg(void);

	public:
		string processing(unsigned char *, size_t);
};

#endif