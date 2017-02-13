.phony all:sws client 

sws: sws.c
	gcc sws.c -o sws
client: client.c
	gcc client.c  -o client

.PHONY clean:
clean:
	-rm -rf *.o *.exe
