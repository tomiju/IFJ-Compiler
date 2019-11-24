#include "symtable.h"
#include <stdint.h>
#include <stdio.h>
#include "errors.h"

int zero = 0;
int one = 1;
int two = 2;
int three = 3;
int minus_two = -2;

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

int htab_insert(htab_t *t, char *key, void* val, int type, int frame, bool isConst, bool isLabel, bool defined){
	//htab_item_t* item = htab_find(t, key);
	
	// ak sa už nachádza v tabuľke, zmenia sa hodnoty
	/*if(item != NULL){
		item->value = val;
		item->type = type;
		item->frame = frame;
		item->isConst = isConst;
		item->isLabel = isLabel;
		item->defined = defined;
		return 0;
	}*/

	htab_item_t *ht_item = malloc(sizeof(*ht_item));

	if(ht_item == NULL){
		return INTERNAL_ERROR;
	}

	ht_item->key = malloc(sizeof(strlen(key)) + 1);

	if(ht_item->key == NULL){
		return INTERNAL_ERROR;
	}

	unsigned idx = htab_hash_function(key);

	switch(type){
		case INT: case FUNC: ht_item->value.ival = val; break;
		case FLOAT: ht_item->value.dval = val; break;
		case STRING: case NIL: case BOOL: case TYPE_NAME: ht_item->value.sval = val; break;
	}

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
	htab_insert(htab, "inputs", &zero, FUNC, 0, false, true, true);
	htab_insert(htab, "inputi", &zero, FUNC, 0, false, true, true);
	htab_insert(htab, "inputf", &zero, FUNC, 0, false, true, true);
	htab_insert(htab, "len", &one, FUNC, 0, false, true, true);
	htab_insert(htab, "substr", &three, FUNC, 0, false, true, true);
	htab_insert(htab, "ord", &two, FUNC, 0, false, true, true);
	htab_insert(htab, "chr", &one, FUNC, 0, false, true, true);
	htab_insert(htab, "print", &minus_two, FUNC, 0, false, true, true);
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
