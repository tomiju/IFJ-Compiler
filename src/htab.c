#include "htab.h"
#include <stdint.h>
#include <stdio.h>

unsigned int htab_hash_function(char *str) {
    uint32_t h=0;     // musí mít 32 bitů (but why ?)
    const unsigned char *p;
    for(p=(const unsigned char*)str; *p!='\0'; p++)
        h = 65599*h + *p;
    return h % SIZE;
}

htab_t *htab_init(){
	htab_t *htab = malloc(sizeof(*htab));

	if(htab != NULL){
		for(unsigned i = 0; i < SIZE; i++){
			htab->ptr[i] = NULL;
		}
	}
	
	return htab;
}

void htab_insert(htab_t *t, char *key, int val){
	htab_item_t *ht_item = malloc(sizeof(*ht_item));

	if(ht_item != NULL){	//TODO NULL
		ht_item->key = malloc(sizeof(strlen(key)) + 1);	//TODO NULL

		/*if(ht_item->key == NULL){
			fprintf(stderr, ("Chyba alokácie"));
		}*/

		unsigned idx = htab_hash_function(key);

		ht_item->value = val;
		strcpy(ht_item->key, key);
		ht_item->next = t->ptr[idx];

		t->ptr[idx] = ht_item;
	}
}

int htab_find(htab_t *t, char *key){
	unsigned idx = htab_hash_function(key);

	htab_item_t *actual;
	htab_item_t *next = t->ptr[idx];

	while(next != NULL){
		actual = next;

		if(strcmp(actual->key, key) == 0){
			return actual->value;
		}

		next = actual->next;
	}

	return -1;
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
