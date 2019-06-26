

CFLAGS		= -Wall -g3 -std=c++17 -pthread -lsqlite3 -lsodium
CC			= g++
NETWORK		= src/network/requests.cpp src/network/socket.cpp
ROUTER		= src/router/find.cpp src/router/garlic.cpp src/router/namespace.cpp \
			src/router/neighbors.cpp src/router/nodes.cpp
STORAGE		= src/storage/clients.cpp src/storage/database.cpp src/storage/garlic.cpp \
			src/storage/neighbors.cpp src/storage/nodes.cpp src/storage/routes.cpp \
			src/storage/tasks.cpp
OTHER		= src/encryption.cpp src/message.cpp


all:
	$(CC) $(ROUTER) $(NETWORK) $(STORAGE) $(OTHER) src/service.cpp -o garlicd $(CFLAGS)

test:
	$(CC) $(ROUTER) $(NETWORK) $(STORAGE) $(OTHER) src/test/main.cpp -o test $(CFLAGS)

clear:
	rm -f garlicd test