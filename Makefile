CC=gcc

all:	client/trivial server/triviald

client/trivial:	trivial.o trivial.h common.o common.h
	$(CC) trivial.o common.o -o client/trivial

server/triviald:	triviald.o triviald.h common.o common.h
	$(CC) triviald.o common.o -o server/triviald

trivial.o:	trivial.c trivial.h common.c common.h

triviald.o:	triviald.c triviald.h common.c common.h

common.o:	common.c common.h

clean:	
	@rm -f ./client/trivial
	@rm -f ./server/*
	@rm -rf *.o
	@rm -rf *~
	@rm -rf *.log
	@rm -rf *.toc
	@rm -rf *.aux
