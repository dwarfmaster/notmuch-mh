PROGS=tree.prog opts_test.prog
OBJS=opts.o
CFLAGS=-Wall -Wextra -g
LDFLAGS=-lnotmuch
CC=gcc

all: $(PROGS)

%.prog : %.o $(OBJS)
	$(CC) $(CFLAGS)    -o $@ $^ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clear :
	@touch $(PROGS) tmp.o
	@rm -v $(PROGS) *.o

rec : clear all

.PHONY: all clear rec


