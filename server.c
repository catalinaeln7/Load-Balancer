/* Copyright 2021 <Popa Catalina-Elena> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"
#include "LinkedList.h"
#include "Hashtable.h"

server_memory* init_server_memory() {
	/* TODO. */
	server_memory *serv_mem;

	serv_mem = malloc(sizeof(server_memory));
	DIE(NULL == serv_mem, "malloc() failed!");

	serv_mem->ht = ht_create(MAX_BUCKETS, hash_function_key,
							 compare_function_strings);

	return serv_mem;
}

void server_store(server_memory* server, char* key, char* value) {
	/* TODO. */
	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

void server_remove(server_memory* server, char* key) {
	/* TODO. */
	ht_remove_entry(server->ht, key);
}

char* server_retrieve(server_memory* server, char* key) {
	/* TODO. */
	return (char*)ht_get(server->ht, key);
}

void free_server_memory(server_memory* server) {
	/* TODO. */
	ht_free(server->ht);
	free(server);
}
