all: ensure_mount compile

exec: kill refresh

refresh: compile run

dir = webfs

ensure_mount:
	[ -d $(dir) ] || mkdir $(dir)

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
