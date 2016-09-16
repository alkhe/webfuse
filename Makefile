all: compile

compile:
	clang `pkg-config --cflags --libs fuse` webfuse.c get.c die.c -owebfuse

run:
	./webfuse webfs

kill:
	fusermount -uz webfs

sample:
	cat webfs/google.com

log:
	cat debug.log
