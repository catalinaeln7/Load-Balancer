/* Copyright 2021 <Popa Catalina-Elena> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "server.h"
#include "LinkedList.h"
#include "Hashtable.h"
#include "utils.h"


// ---FUNCTII AUXILIARE---

// Adauga in hashring si sorteaza in acelasi timp dupa hash
void hashring_add(unsigned int *hashring, int nr, unsigned int tag) {
	int i = nr - 1;

	while (i >= 0 && hash_function_servers(&hashring[i]) >=
		   hash_function_servers(&tag)) {
		if (hash_function_servers(&hashring[i]) ==
		   hash_function_servers(&tag)) {
			if (hashring[i] % 100000 < tag % 100000) {
				break;
			}
		}
		hashring[i + 1] = hashring[i];
		--i;
	}

	hashring[i + 1] = tag;
}

// Sterge din hashring etichetele date ca parametru
void hashring_delete(unsigned int *hashring, int nr, unsigned int tag) {
	for (int i = 0; i < nr; ++i) {
		if (hash_function_servers(&hashring[i]) == hash_function_servers(&tag)) {
			for (int j = i; j < nr - 1; ++j) {
				hashring[j] = hashring[j + 1];
			}
		}
	}
}

// Gaseste id-ul serverului in care va fi stocat un obiect cu cheia key
int find_id_server(load_balancer *main, char *key) {
	int id;

	if (hash_function_key(key) >
		hash_function_servers(&main->hashring[main->hashring_size - 1])) {
		id = (int)(main->hashring[0] % 100000);
	} else if (hash_function_key(key) <
			   hash_function_servers(&main->hashring[0])) {
		id = (int)(main->hashring[0] % 100000);
	} else {
		for (unsigned int i = 0; i < main->hashring_size; ++i) {
			if (hash_function_key(key) >
				hash_function_servers(&main->hashring[i])) {
					id = (int)(main->hashring[i + 1] % 100000);
			}
		}
	}

	return id;
}

// Returneaza nodul care contine serverul cu id-ul dat
ll_node_t *get_server_node(load_balancer *main, int *server_id) {
	unsigned int i, ok = 1;
	ll_node_t *curr, *found;

	curr = main->serv_addr->head;

	for (i = 0; i < main->serv_addr->size && ok == 1; ++i) {
		if ((*(server_memory*)curr->data).server_id == *server_id) {
			found = curr;
			ok = 0;
		}
		curr = curr->next;
	}

	return found;
}

// Returneaza pozitia in lista a serverului cu id-ul dat
int get_server_position(load_balancer *main, int *server_id) {
	int cnt = 0;
	ll_node_t *curr;

	curr = main->serv_addr->head;
	while ((*(server_memory*)curr->data).server_id != *server_id) {
		cnt++;
		curr = curr->next;
	}

	return cnt;
}

// Redistribuirea obiectelor dupa add_server
void rebalance_add(load_balancer *main, int *server_id) {
	char *key, *value;
	unsigned int i, j, k, pos = -1, next_serv_pos;
	int next_id = 0, id_aux = 0;
	ll_node_t *curr, *tmp, *next;
	server_memory *my_server;

	for (i = 0; i < 3; ++i) {
		int ok = 0;
		for (j = pos + 1; j < main->hashring_size && ok == 0; ++j) {
			if ((int)(main->hashring[j] % 100000) == *server_id) {
				pos = j;
				ok = 1;
			}
		}
		// Gasesc pozitia urmatorului server celui adaugat
		next_serv_pos = pos + 1;
		while ((int)(main->hashring[next_serv_pos] % 100000) == *server_id) {
			next_serv_pos++;
		}
		next_serv_pos = next_serv_pos % main->hashring_size;
		next_id = main->hashring[next_serv_pos] % 100000;

		tmp = get_server_node(main, &next_id);
		my_server = &(*(server_memory*)tmp->data);

		// Ma pozitionez in hashtable
		for (k = 0; k < MAX_BUCKETS; ++k) {
			curr = my_server->ht->buckets[k]->head;
			while (curr) {
				next = curr->next;
				key = (*(info*)curr->data).key;
				value = (*(info*)curr->data).value;
				loader_store(main, key, value, &id_aux);
				if (id_aux != next_id) {
					server_remove(my_server, key);
				}
				curr = next;
			}
		}
	}
}

// Redistribuirea obiectelor dupa remove_server
void rebalance_remove(load_balancer *main, int *server_id) {
	int pos, id_aux = 0;
	char *key, *value;
	ll_node_t *prev, *rem, *curr, *next;
	server_memory *rem_server;

	pos = get_server_position(main, server_id);
	rem = get_server_node(main, server_id);
	rem_server = &(*(server_memory*)rem->data);

	// Refac legaturile in lista inlantuita
	if (pos == 0) {
		main->serv_addr->head = main->serv_addr->head->next;
	} else {
		prev = get_nth_node(pos - 1, main->serv_addr);
		prev->next = prev->next->next;
	}

	// Ma pozitionez in hashtable
	for (int i = 0; i < MAX_BUCKETS; ++i) {
		curr = rem_server->ht->buckets[i]->head;
		while (curr) {
			next = curr->next;
			key = (*(info*)curr->data).key;
			value = (*(info*)curr->data).value;
			loader_store(main, key, value, &id_aux);
			curr = next;
		}
	}

	free_server_memory(rem_server);
	free(rem);
	main->serv_addr->size--;
}

// ---FUNCTII LOAD BALANCER---

load_balancer* init_load_balancer() {
	/* TODO. */
	load_balancer *main = malloc(sizeof(load_balancer));
	DIE(NULL == main, "malloc() failed");

	main->serv_addr = ll_create(sizeof(server_memory));
	main->hashring_size = 0;

	for (int i = 0; i < NMAX; ++i) {
		main->hashring[i] = 0;
	}

	main->nr_obj = 0;

	return main;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
	/* TODO. */
	ll_node_t *curr;
	server_memory *my_server;

	*server_id = find_id_server(main, key);
	curr = get_server_node(main, server_id);
	my_server = &(*(server_memory*)curr->data);

	server_store(my_server, key, value);
	main->nr_obj++;
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	/* TODO. */
	ll_node_t *curr;
	server_memory *my_server;

	*server_id = find_id_server(main, key);
	curr = get_server_node(main, server_id);
	my_server = &(*(server_memory*)curr->data);

	return server_retrieve(my_server, key);
}

void loader_add_server(load_balancer* main, int server_id) {
	/* TODO. */
	unsigned int tag[3], i;
	server_memory *my_server;

	my_server = init_server_memory();
	my_server->server_id = server_id;

	// Adaugare in hashring 3 replici ale serverului

	for (i = 0; i < 3; ++i) {
		tag[i] = i * 100000 + server_id;
		hashring_add(main->hashring, main->hashring_size, tag[i]);
		main->hashring_size++;
	}

	ll_add_nth_node(main->serv_addr, main->serv_addr->size, my_server);

	// Redistribuie obiectele

	if (main->serv_addr->size > 0) {
		rebalance_add(main, &server_id);
	}

	free(my_server);
}

void loader_remove_server(load_balancer* main, int server_id) {
	/* TODO. */
	unsigned int tag[3], i;

	for (i = 0; i < 3; ++i) {
		tag[i] = i * 100000 + server_id;
		hashring_delete(main->hashring, main->hashring_size, tag[i]);
		main->hashring_size--;
	}

	// Redistribuie obiectele

	if (main->serv_addr->size > 0 && main->nr_obj > 0) {
		rebalance_remove(main, &server_id);
	}
}

void free_load_balancer(load_balancer* main) {
	/* TODO. */
	ll_node_t *curr = main->serv_addr->head;
	ll_node_t *next;

	for (unsigned int i = 0; i < main->serv_addr->size; ++i) {
		next = curr->next;
		free_server_memory(&(*(server_memory*)curr->data));
		free(curr);
		curr = next;
	}

	free(main->serv_addr);
	free(main);
}
