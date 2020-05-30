
test:
	g++ -Wall -g3 -std=c++17 -o spruce src/spruce.cpp src/pack.cpp src/encryption.cpp src/functions.cpp src/tcptunnel.cpp \
	src/handler.cpp src/network.cpp src/userapi.cpp src/storage/*.cpp src/router/*.cpp -lsodium -lsqlite3 -pthread
	
debug:
	g++ -Wall -g3 -std=c++17 -o debug debug.cpp src/pack.cpp src/encryption.cpp src/functions.cpp src/tcptunnel.cpp \
	src/handler.cpp src/network.cpp src/storage/*.cpp src/router/*.cpp -lsodium -lsqlite3 -pthread