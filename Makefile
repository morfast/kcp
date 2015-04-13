all: server client

server: udp_server.c
	gcc -O0 -g -o server udp_server.c

client: udp_client.c
	gcc -O0 -g -o client udp_client.c

clean:
	rm server client


