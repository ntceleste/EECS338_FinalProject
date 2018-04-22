all:
	gcc client.c -o p1 -pthread -lrt -lpthread -lm
	gcc server.c -o p2 -pthread -lrt -lpthread -lm
