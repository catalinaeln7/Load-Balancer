/* Copyright 2021 <Popa Catalina-Elena> */
#ifndef SERVER_H_
#define SERVER_H_

#define MAX_BUCKETS 100

#include "Hashtable.h"

typedef struct server_memory server_memory;
struct server_memory {
	/* TODO */
	hashtable_t *ht;
	int server_id;
};

server_memory* init_server_memory();

void free_server_memory(server_memory* server);

/**
 * server_store() - Stores a key-value pair to the server.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 */
void server_store(server_memory* server, char* key, char* value);

/**
 * server_remove() - Removes a key-pair value from the server.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 */
void server_remove(server_memory* server, char* key);

/**
 * server_retrieve() - Gets the value associated with the key.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 *
 * Return: String value associated with the key
 *         or NULL (in case the key does not exist).
 */
char* server_retrieve(server_memory* server, char* key);

#endif  /* SERVER_H_ */
