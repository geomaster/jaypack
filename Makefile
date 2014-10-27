FASTFLAGS=-O3 -march=native
DEBUGFLAGS=-O0 -g
JPFLAGS=-Wall

default: jaypack jaypack-server jaypack-client

debug: jaypack-dbg jaypack-server-dbg jaypack-client-dbg

jaypack: jaypack.c
	$(CC) -ojaypack $(FASTFLAGS) $(JPFLAGS) $(CFLAGS) jaypack.c

jaypack-dbg: jaypack.c
	$(CC) -ojaypack-dbg $(DEBUGFLAGS) $(JPFLAGS) $(CFLAGS) jaypack.c

jaypack-client: jaypack-client.c
	$(CC) -ojaypack-client $(FASTFLAGS) $(JPFLAGS) $(CFLAGS) jaypack-client.c

jaypack-server: jaypack-server.c
	$(CC) -ojaypack-server $(FASTFLAGS) $(JPFLAGS) $(CFLAGS) jaypack-server.c

jaypack-client-dbg: jaypack-client.c
	$(CC) -ojaypack-client-dbg $(DEBUGFLAGS) $(JPFLAGS) $(CFLAGS) jaypack-client.c

jaypack-server-dbg: jaypack-server.c
	$(CC) -ojaypack-server-dbg $(DEBUGFLAGS) $(JPFLAGS) $(CFLAGS) jaypack-server.c

clean: 
	rm -f jaypack jaypack-dbg jaypack-client jaypack-client-dbg jaypack-server \
		jaypack-server-dbg

.PHONY: clean debug
