#ifndef __HTABLE_H__
#define __HTABLE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// veľkosť tabuľky
#define SIZE 100

// položky
typedef struct htab_item {
	int value;
	char *key;
	bool isFunc;
	struct htab_item *next;
} htab_item_t;

// tabuľka
typedef struct htab {
	struct htab_item* ptr[SIZE]; 
} htab_t;           

// vloží vstavané funkcie do hash table
void htab_insert_default_functions(htab_t *htab);

// hashovacia funkcia, vracia index do tabuľky
unsigned int htab_hash_function(char *str);

// inicializuje tabuľku predanú ukazovateľom
// pri úspechu vracia 0, pri chybe INTERNAL_ERROR
int htab_init(htab_t **htab);           

// vloží do tabuľky t prvok s klúčom key, hodnotou val 
// a príznakom funkcie
// pri úspechu vracia 0, pri chybe INTERNAL_ERROR
int htab_insert(htab_t *t, char *key, int val, bool isFunc);

// vyhľadá a vráti ukazateľ na konkrétny prvok
// ak sa nenašiel, vráti NULL
htab_item_t* htab_find(htab_t *t, char *key);

// vyčistí tabuľku (tabuľka je po vykonaní prázdna)
void htab_clear(htab_t * t);	

// uvoľní tabuľku z pamäte (volá htab_clear)
void htab_free(htab_t * t);		

#endif // __HTABLE_H__
