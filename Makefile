
main:
	g++ -Wall -g3 -std=c++17 -o spruce src/api.cpp src/encryption.cpp src/functions.cpp src/handler.cpp src/network.cpp src/pack.cpp \
	src/tcptunnel.cpp src/spruce.cpp src/router/*.cpp src/storage/*.cpp -lsodium -lsqlite3 -pthread
	
