/* Copyright 2021 <Popa Catalina-Elena> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"
#include "LinkedList.h"

#define MAX_BUCKET_SIZE 64

/*
 * Functii de comparare a cheilor:
 */
int
compare_function_uints(void *a, void *b)
{
	unsigned int uint_a = *((unsigned int *)a);
	unsigned int uint_b = *((unsigned int *)b);

	if (uint_a == uint_b) {
		return 0;
	} else if (uint_a < uint_b) {
		return -1;
	} else {
		return 1;
	}
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functii de hashing:
 */
// Functia hash pentru servere
unsigned int hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

// Functia hash pentru chei
unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *) a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

void free_hash(info *item) {
    free(item->key);
    free(item->value);
    free(item);
}

info *create_item(void *key, void *value, unsigned int key_size,
				  unsigned int value_size)
{
	info* item = malloc(sizeof(info));
	DIE(NULL == item, "malloc() failed!");

    item->key = malloc(key_size);
	DIE(NULL == item->key, "malloc() failed!");

    item->value = malloc(value_size);
	DIE(NULL == item->value, "malloc() failed!");

    memcpy(item->key, key, key_size);
    memcpy(item->value, value, value_size);

    return item;
}

/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	/* TODO */
	unsigned int i;

	hashtable_t *table = malloc(sizeof(*table));
	DIE(NULL == table, "malloc() failed!");

	table->size = 0;
	table->hmax = hmax;
	table->hash_function = hash_function;
	table->compare_function = compare_function;
	table->buckets = malloc(table->hmax * sizeof(*table->buckets));
	DIE(NULL == table->buckets, "malloc() failed!");

    for (i = 0; i < table->hmax; ++i) {
        table->buckets[i] = ll_create(sizeof(info));
	}

	return table;
}

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune tipul ei), in momentul in care
 * se creeaza o noua intrare in hashtable (in cazul in care cheia nu se gaseste deja in ht), trebuie creata o data_node->key_cpy
 * a valorii la care pointeaza key si adresa acestei copii trebuie salvata in structura info asociata intrarii din ht.
 * Pentru a sti cati octeti trebuie alocati si copiati, folositi parametrul key_size_bytes.
 *
 * Motivatie:
 * Este nevoie sa data_node->key_cpym valoarea la care pointeaza key deoarece dupa un apel put(ht, key_actual, value_actual),
 * valoarea la care pointeaza key_actual poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia unei intrari din hashtable.
 * Nu ne dorim acest lucru, fiindca exista riscul sa ajungem in situatia in care nu mai stim la ce cheie este
 * inregistrata o anumita valoare.
 */

void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	ll_node_t *curr;
	ll_node_t *new = malloc(sizeof(*new));
	info *data_node = malloc(sizeof(*data_node));
	unsigned int hash_index = ht->hash_function(key) % ht->hmax;

	DIE(NULL == new, "malloc() failed!");
	DIE(NULL == data_node, "malloc() failed!");

	data_node->key = malloc(key_size);
	DIE(NULL == data_node->key, "malloc() failed!");
	memcpy(data_node->key, key, key_size);

	data_node->value = malloc(value_size);
	DIE(NULL == data_node->value, "malloc() failed!");
	memcpy(data_node->value, value, value_size);

	curr = ht->buckets[hash_index]->head;

	if (curr == NULL) {
		new->data = data_node;
		new->next = NULL;

		ht->buckets[hash_index]->head = new;
		ht->buckets[hash_index]->size++;
		ht->size++;
	} else {
		do {
			if (ht->compare_function((*(info*)(curr->data)).key, data_node->key) == 0) {
				memcpy((*(info*)(curr->data)).value, data_node->value, value_size);
				free_hash(data_node);
				free(new);
				return;
			}
			if (curr->next != NULL) {
				curr = curr->next;
			}
		} while (curr->next != NULL);

		ll_add_nth_node(ht->buckets[hash_index],
						ht->buckets[hash_index]->size, data_node);
		free(data_node);
		free(new);
		ht->size++;
	}
}

void *
ht_get(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* node = ht->buckets[index]->head;

	while(node != NULL) {
		if (ht->compare_function((*(info*)(node->data)).key, key) == 0) {
			return (*(info*)(node->data)).value;
		}
		node = node->next;
	}

	return NULL;
}

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable folosind functia put
 * 0, altfel.
 */
int
ht_has_key(hashtable_t *ht, void *key)
{
	/* TODO */
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* node = ht->buckets[index]->head;

	while(node != NULL) {
		if(ht->compare_function((*(info*)(node->data)).key, key) == 0) {
			return 1;
		}
		node = node->next;
	}

	return 0;
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o intrare din hashtable (adica memoria
 * pentru copia lui key --vezi observatia de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */


void
ht_remove_entry(hashtable_t *ht, void *key)
{
	/* TODO */

	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t* node = ht->buckets[index]->head;
	ll_node_t* prev = NULL;

	while(node != NULL)
	{
		if(ht->compare_function((*(info*)(node->data)).key, key) == 0)
		{
			if(prev != NULL) {
				prev->next = node->next;
			} else {
				ht->buckets[index]->head = node->next;
			}
			free_hash(node->data);
			free(node);
			ht->buckets[index]->size--;
			ht->size--;
			return;
		}
		prev = node;
		node = node->next;
	}
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable, dupa care elibereaza si memoria folosita
 * pentru a stoca structura hashtable.
 */
void
ht_free(hashtable_t *table)
{
	/* TODO */
	unsigned int i, j;
	ll_node_t *curr, *next;

	for (i = 0; i < table->hmax; ++i) {
		curr = table->buckets[i]->head;
		for (j = 0; j < table->buckets[i]->size; ++j) {
			next = curr->next;
			free_hash(curr->data);
			free(curr);
			curr = next;
		}
		free(table->buckets[i]);
	}

    free(table->buckets);
    free(table);
}

unsigned int
ht_get_size(hashtable_t *ht)
{
	if (ht == NULL) {
		return 0;
	}

	return ht->size;
}

unsigned int
ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL) {
		return 0;
	}

	return ht->hmax;
}
