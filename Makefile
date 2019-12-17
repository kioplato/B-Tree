#!/usr/bin/make -f

SHELL = /bin/sh

objdir = build
bindir = bin
srcdir = src
libdir = lib

CC = gcc
CFLAGS = -Wall -Wextra -g -O2
LDFLAGS = -g -L $(libdir) -Wl,-rpath=$(libdir) -lbf

SRCS = $(shell find $(srcdir) -name "*.c")
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: main1 main2 main3

tests: db RD_Split_Records ib insert

main1: $(OBJS) examples/main1.o
	$(CC) $(LDFLAGS) $(addprefix $(objdir)/, $(notdir $^)) -o $(bindir)/$@

main2: $(OBJS) examples/main2.o
	$(CC) $(LDFLAGS) -o $(bindir)/$@ $(addprefix $(objdir)/, $(notdir $^))

main3: $(OBJS) examples/main3.o
	$(CC) $(LDFLAGS) -o $(bindir)/$@ $(addprefix $(objdir)/, $(notdir $^))

db: $(OBJS) tests/main_test_DB.o
	$(CC) $(LDFLAGS) -o $(bindir)/$@ $(addprefix $(objdir)/, $(notdir $^))

RD_Split_Records: $(OBJS) tests/test_RD_Split_Records.o
	$(CC) $(LDFLAGS) -o $(bindir)/$@ $(addprefix $(objdir)/, $(notdir $^))

ib: $(OBJS) tests/test_indexblock.o
	$(CC) $(LDFLAGS) -o $(bindir)/$@ $(addprefix $(objdir)/, $(notdir $^))

insert: $(OBJS) tests/test_AM_insert1.o
	$(CC) $(LDFLAGS) -o $(bindir)/$@ $(addprefix $(objdir)/, $(notdir $^))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(objdir)/$(notdir $@)

.PHONY: clean
clean:
	rm -f $(objdir)/* $(bindir)/*
	rm -f ./EMP-AGE ./EMP-SAL ./EMP-NAME ./EMP-DNAME ./EMP-FAULT


#main1:
#	@echo " Compile main1 ...";
#	gcc -L ./lib/ -Wl,-rpath,./lib/ ./examples/main1.c ./src/AM.c -lbf -o ./build/main1
#
#main2:
#	@echo " Compile main2 ...";
#	gcc -L ./lib/ -Wl,-rpath,./lib/ ./examples/main2.c ./src/AM.c -lbf -o ./build/main2
#
#main3:
#	@echo " Compile main3 ...";
#	gcc -L ./lib/ -Wl,-rpath,./lib/ ./examples/main3.c ./src/AM.c -lbf -o ./build/main3
#
#bf:
#	@echo " Compile bf_main ...";
#	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner -O2
