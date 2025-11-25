CC = gcc
CFLAGS = -Wall -g

all: serverTCP clientTCP

serverTCP: serverTCP.c common.c common.h
	$(CC) $(CFLAGS) -o serverTCP serverTCP.c common.c

clientTCP: clientTCP.c common.c common.h
	$(CC) $(CFLAGS) -o clientTCP clientTCP.c common.c

clean:
	rm -f serverTCP clientTCP server_info.txt users.txt

run-server: serverTCP
	./serverTCP 127.0.0.1 8082

run-client: clientTCP
	./clientTCP

.PHONY: all clean run-server run-client