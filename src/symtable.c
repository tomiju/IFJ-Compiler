/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   symtable.c
 *
 *
 * Datum:    30.11.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta    <xkucht09@stud.fit.vutbr.cz>
 */

#include "symtable.h"
#include <stdint.h>
#include <stdio.h>
#include "errors.h"

htab_item_t* htab_find(htab_t *t, char *key){
	unsigned idx = htab_hash_function(key);

	htab_item_t *actual;
	htab_item_t *next = t->ptr[idx];

	while(next != NULL){
		actual = next;

		if(strcmp(actual->key, key) == 0){
			if(actual->type != FUNC){
				actual->reviewed += 1;
			}
			return actual;
		}

		next = actual->next;
	}

	return NULL;
}

int htab_insert(htab_t *t, char *key, int type, int frame, bool isConst, bool isLabel, bool defined){
	htab_item_t *ht_item = malloc(sizeof(*ht_item));

	if(ht_item == NULL){
		return INTERNAL_ERROR;
	}

	ht_item->key = malloc(strlen(key) + 1);

	if(ht_item->key == NULL){
		return INTERNAL_ERROR;
	}

	unsigned idx = htab_hash_function(key);

	ht_item->type = type;
	ht_item->frame = frame;
	ht_item->isConst = isConst;
	ht_item->isLabel = isLabel;
	ht_item->reviewed = false;
	ht_item->defined = defined;
	ht_item->local_vars = NULL;
	strcpy(ht_item->key, key);
	ht_item->next = t->ptr[idx];

	t->ptr[idx] = ht_item;

	return 0;
}

void htab_insert_default_functions(htab_t *htab){
	htab_insert(htab, "inputs", FUNC, 0, false, true, true);
	htab_item_t* inputs = htab_find(htab, "inputs");
	inputs->ival = 0;
	htab_insert(htab, "inputi", FUNC, 0, false, true, true);
	htab_item_t* inputi = htab_find(htab, "inputi");
	inputi->ival = 0;
	htab_insert(htab, "inputf", FUNC, 0, false, true, true);
	htab_item_t* inputf = htab_find(htab, "inputf");
	inputf->ival = 0;
	htab_insert(htab, "len", FUNC, 0, false, true, true);
	htab_item_t* len = htab_find(htab, "len");
	len->ival = 1;
	htab_insert(htab, "substr", FUNC, 0, false, true, true);
	htab_item_t* substr = htab_find(htab, "substr");
	substr->ival = 3;
	htab_insert(htab, "ord", FUNC, 0, false, true, true);
	htab_item_t* ord = htab_find(htab, "ord");
	ord->ival = 2;
	htab_insert(htab, "chr", FUNC, 0, false, true, true);
	htab_item_t* chr = htab_find(htab, "chr");
	chr->ival = 1;
	htab_insert(htab, "print", FUNC, 0, false, true, true);
	htab_item_t* print = htab_find(htab, "print");
	print->ival = -2;
}

unsigned int htab_hash_function(char *str) {
    uint32_t h=0;     // musí mít 32 bitů (but why ?)
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
        h = 65599*h + *p;
    return h % SIZE;
}

int htab_init(htab_t **htab){
	*htab = malloc(sizeof(struct htab));

	if(htab == NULL){
		return INTERNAL_ERROR;
	}

	for(unsigned i = 0; i < SIZE; i++){
		(*htab)->ptr[i] = NULL;
	}

	htab_insert_default_functions(*htab);

	return 0;
}

void htab_clear(htab_t* t){
	if(t != NULL){
		for(unsigned i = 0; i < SIZE; i++){
			if(t->ptr[i] != NULL){
				htab_item_t* item;
				htab_item_t* item_next = t->ptr[i];

				do{
					item = item_next;
					item_next = item->next;
					htab_free(item->local_vars);
					free(item->key);
					free(item);
				} while (item_next != NULL);

				t->ptr[i] = NULL;
			}
		}
	}
}

void htab_free(htab_t* t){
	if(t != NULL){
		htab_clear(t);
		free(t);
	}
}
