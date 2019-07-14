

CC						=	gcc
AR						=	ar
ARFLAGS				=	rvs
CFLAGS				= -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -pedantic -g
OPTFLAGS			= -o3
LDFLAGS				=	-L.
INCLUDES			= -I.
LIBS 					= -pthread



TARGETS 			=	server \
								client


OBJECTSSERVER	=	server_library.o \
								util.o \
								queue.o 


OBJECTSCLIENT = client_library.o \
								util.o


INCLUDE_SERVER=	util.h \
								server_library.h \
								queue.h


INCLUDE_CLIENT= client_library.h \
								util.h


.PHONY: all clean test

.SUFFIXES: .c .h


%: %.c 
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


all: $(TARGETS)


server: server.o libserver.a $(INCLUDE_SERVER)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) 


client: client.o libclient.a $(INCLUDE_CLIENT)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $(LDFLAGS) -o $@ $^  


libserver.a: $(OBJECTSSERVER)
	$(AR) $(ARFLAGS) $@ $^


libclient.a: $(OBJECTSCLIENT)
	$(AR) $(ARFLAGS) $@ $^


clean:
	rm -rf $(TARGETS) *.o *.a data objstore.sock testout.log

test: 
	./server & \
	bash test.sh > testout.log 2>&1 
	sh testsum.sh
	
