/* Copyright 2021 <Popa Catalina-Elena> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LinkedList.h"
#include "utils.h"

linked_list_t*
ll_create(unsigned int data_size)
{
	linked_list_t *list = malloc(sizeof(*list));
	DIE(NULL == list, "malloc() failed!");

	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;

	return list;
}

static ll_node_t *create_node(const void* new_data, unsigned int data_size)
{
	ll_node_t *node = malloc(sizeof(*node));
	DIE(NULL == node, "malloc() failed!");

	node->data = malloc(data_size);
	DIE(NULL == node->data, "malloc() failed!");

	memcpy(node->data, new_data, data_size);

	return node;
}

static int minim(unsigned int a, unsigned int b)
{
	if (a < b)
		return a;
	else
		return b;
}

ll_node_t *get_nth_node(unsigned int n, linked_list_t *list)
{
	ll_node_t *node = list->head;

	for (unsigned int i = 0; i < n; i++)
		node = node->next;

	return node;
}

/*
 * Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e
 * adaugat pe pozitia n a listei reprezentata de pointerul list. Pozitiile din
 * lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla pe
 * pozitia n=0). Daca n >= nr_noduri, noul nod se adauga la finalul listei. Daca
 * n < 0, eroare.
 */
void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
	if (!list)
		return;

	ll_node_t *new_node = create_node(new_data, list->data_size);

	if (!n || !list->size) {
		new_node->next = NULL;
		list->head = new_node;
	} else if (n > 0) {
		ll_node_t *prev_node = get_nth_node(n - 1, list);
		new_node->next = prev_node->next;
		prev_node->next = new_node;
	}

	list->size++;
}

/*
 * Elimina nodul de pe pozitia n din lista al carei pointer este trimis ca
 * parametru. Pozitiile din lista se indexeaza de la 0 (i.e. primul nod din
 * lista se afla pe pozitia n=0). Daca n >= nr_noduri - 1, se elimina nodul de
 * la finalul listei. Daca n < 0, eroare. Functia intoarce un pointer spre acest
 * nod proaspat eliminat din lista. Este responsabilitatea apelantului sa
 * elibereze memoria acestui nod.
 */
void
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
	ll_node_t *tmp, *curr;
	curr = list->head;

	/* Lista este goala, deci nu avem ce elimina */
	if (curr == NULL)
		return;

	if (n <= 0) {
		curr = list->head;
		list->head = curr->next;
		free(curr->data);
		free(curr);
	} else {
		curr = get_nth_node(minim(n - 1, list->size - 1), list);
		tmp = curr->next;
		curr->next = curr->next->next;
		free(tmp->data);
		free(tmp);
	}

	list->size--;
}

/*
 * Functia intoarce numarul de noduri din lista al carei pointer este trimis ca
 * parametru.
 */
unsigned int
ll_get_size(linked_list_t* list)
{
	return list->size;
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
 * sfarsit, elibereaza memoria folosita de structura lista si actualizeaza la
 * NULL valoarea pointerului la care pointeaza argumentul (argumentul este un
 * pointer la un pointer).
 */
void
ll_free(linked_list_t** pp_list)
{
	ll_node_t *curr = (*pp_list)->head;
	while (curr != NULL)
	{
		ll_node_t *tmp = curr;
		curr = curr->next;
		free(tmp->data);
		free(tmp);
	}
	free(*pp_list);
	*pp_list = NULL;
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM
 * ca stocheaza int-uri. Functia afiseaza toate valorile int stocate in nodurile
 * din lista inlantuita separate printr-un spatiu.
 */
void
ll_print_int(linked_list_t* list)
{
	ll_node_t* curr;

	if (list == NULL) {
		return;
	}

	curr = list->head;
	for (unsigned int i = 0; i < list->size; ++i) {
		printf("%d ", *((int*)curr->data));
		curr = curr->next;
	}

	printf("\n");
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM
 * ca stocheaza string-uri. Functia afiseaza toate string-urile stocate in
 * nodurile din lista inlantuita, separate printr-un spatiu.
 */
void
ll_print_string(linked_list_t* list)
{
	ll_node_t *curr;
	curr = list->head;

	while (curr != NULL) {
		printf("%s ", (char*)curr->data);
		curr = curr->next;
	}
	printf("\n");
}
