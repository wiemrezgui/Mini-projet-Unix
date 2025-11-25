CC = gcc
CFLAGS = -Wall -g

all: serveurTCP clientTCP

serveurTCP: serveurTCP.c common.h
	$(CC) $(CFLAGS) -o serveurTCP serveurTCP.c

clientTCP: clientTCP.c common.h
	$(CC) $(CFLAGS) -o clientTCP clientTCP.c

clean:
	rm -f serveurTCP clientTCP server_info.txt users.txt

run-server: serveurTCP
	./serveurTCP 127.0.0.1 8082

run-client: clientTCP
	./clientTCP

.PHONY: all clean run-server run-client