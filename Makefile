CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/network.c src/epoll_utils.c src/game.c src/client.c src/server.c src/error_handle.c

OBJ = obj/network.o obj/epoll_utils.o obj/game.o obj/client.o obj/server.o obj/error_handle.o

SERVER = bin/gameserver
CLIENT = bin/gameclient

all: $(SERVER) $(CLIENT)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER): $(OBJ) apps/gameserver.c
	$(CC) $(CFLAGS) $^ -o $(SERVER)

$(CLIENT): $(OBJ) apps/gameclient.c
	$(CC) $(CFLAGS) $^ -o $(CLIENT)

clean:
	rm -f obj/*.o bin/*

.PHONY: all clean
