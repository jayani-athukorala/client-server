#Makefile for source files : server.c client.c

all:
	find . -type f -executable -exec rm '{}' \;
	rm -f objects/*.o
	gcc src/server_v3.c -o server
	gcc src/client.c -o client

	rm -f *.txt
	rm -rf results/
	rm -f log/*.txt
	mkdir results/