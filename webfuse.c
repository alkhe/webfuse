#include <stdio.h>
#include <errno.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <string.h>
#include <search.h>

#include "get.h"
#include "die.h"

#define STR_EQ(s, t) (strcmp((s), (t)) == 0)
#define DEBUG(s) fputs(s, DEBUG_FD); fflush(DEBUG_FD)
#define DEBUGF(f, ...) fprintf(DEBUG_FD, f, __VA_ARGS__); fflush(DEBUG_FD)

static FILE* DEBUG_FD;

static int wf_getattr(const char* path, struct stat *st) {

	DEBUGF("stat %s\n", path);

	if (STR_EQ(path, "/")) {
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
		return 0;
	}

	const char* plain_path = path + 1;

	if (
		STR_EQ(plain_path, ".Trash") ||
		STR_EQ(plain_path, ".Trash-1000") ||
		STR_EQ(plain_path, "AACS") ||
		STR_EQ(plain_path, ".xdg-volume-info") ||
		STR_EQ(plain_path, "BDSVM") ||
		STR_EQ(plain_path, "autorun.inf") ||
		STR_EQ(plain_path, "BDMV")
	   ) {
		return -ENOENT;
	}


	st->st_mode = S_IFREG | 0444;
	st->st_nlink = 1;
	st->st_size = 0;
	return 0;
}

static int wf_readdir(const char* path, void* buf, fuse_fill_dir_t fill,
	off_t offset, struct fuse_file_info* info) {
	
	DEBUGF("ls %s\n", path);

	fill(buf, ".", NULL, 0);
	fill(buf, "..", NULL, 0);
	fill(buf, "google.com", NULL, 0);

	return 0;
}

static int wf_open(const char* path, struct fuse_file_info* info) {

	DEBUGF("open %s\n", path);

	return 0;
}

static int wf_read(const char* path, char* buf, size_t size, off_t offset,
	struct fuse_file_info* info) {
	
	DEBUGF("read %s\n", path);

	struct entry query;
	query.key = (char*)(path);

	struct entry* document_entry = hsearch(query, FIND);

	if (document_entry == NULL) {
		document_entry = hsearch(query, ENTER);
		document_entry->data = (char*)(perform_get(path + 1, 80));
	}

	const char* document = (const char*)(document_entry->data);

	size_t bytes_available = strlen(document + offset);
	size_t bytes_copyable = bytes_available < size ? bytes_available : size;

	memcpy(buf, document + offset, bytes_copyable);

	DEBUGF("total %zu b\n", bytes_available);	
	DEBUGF("copied %zu b\n", bytes_copyable);
	DEBUGF("size %zu b\n", size);
	DEBUGF("offset %zu b\n", offset);
	// DEBUG(document);

	return bytes_copyable;
}

/*
struct cache_entry {
	const char* key;
	const char* value;
	struct cache_entry* prev;
	struct cache_entry* next;
};

#define UNSIGNED_MOD(a, b) ((a) < 0 ? ((a) % (b)) + (b) : (a) % (b))

static struct cache_entry* create_cyclic_cache(int entries) {
	size_t cache_size = sizeof(struct cache_entry) * entries;

	struct cache_entry* cache = (struct cache_entry*)(malloc(cache_size));
	memset(cache, 0, cache_size);

	for (size_t i = 0; i < entries; i++) {
		cache[i].prev = cache + UNSIGNED_MOD(i - 1, entries);
		cache[i].next = cache + (i + 1) % entries;
	}

	return cache;
}

static struct cache_entry* find_entry_cyclic(struct cache_entry* cache, const char* key) {
	struct cache_entry* pointer = cache;

	do {
		if (STR_EQ(pointer->key, key)) {
			return pointer;
		}
		pointer++;
	} while (pointer->next != cache);

	return NULL;
}

static struct cache_entry* push_entry(struct cache_entry* cache, const char* key, const char* value) {
	struct cache_entry* next = cache->next;

	next->key = key;
	next->value = value;
	
	return next;
}

static const int CACHE_ENTRIES = 8;
*/

static struct fuse_operations wf_operations = {
	.getattr = wf_getattr,
	.readdir = wf_readdir,
	.open = wf_open,
	.read = wf_read
};

static const int HASH_ENTRIES = 128;

int main(int argc, char** argv) {
	DEBUG_FD = fopen("debug.log", "w");
	DEBUG("init\n");

	int create_result = hcreate(HASH_ENTRIES);
	if (create_result == 0) {
		die("error: couldn't create hash table");
	}
	return fuse_main(argc, argv, &wf_operations, NULL);
}
