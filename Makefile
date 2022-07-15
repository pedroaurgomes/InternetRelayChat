cc     = gcc      
MAKE   = make
RM     = rm
SERVER = ./server.c ./utils.c
CLIENT = ./client.c ./utils.c
CFLAGS = -Wall -Werror -pthread

#targets .
all: client.c server.c
	$(cc) -o client $(CLIENT) $(CFLAGS)
	$(cc) -o server $(SERVER) $(CFLAGS)

# target to initiate server side
server_side:
	./server

# target to initiate client side
client_side:
	./client

clean: server client
	$(RM) server
	$(RM) client

zip: 
	zip -v Redes_entrega_1.zip README.md Makefile client.c server.c utils.c utils.h