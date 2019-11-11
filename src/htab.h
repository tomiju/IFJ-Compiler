#ifndef __HTABLE_H__
#define __HTABLE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// TODO zmeniť 20 iba na test
#define SIZE 2

// položky
typedef struct htab_item {
	unsigned value;
	char *key;
	struct htab_item *next;
} htab_item_t;

// tabuľka
typedef struct htab {
	struct htab_item* ptr[SIZE]; 
} htab_t;           

// hashovacia funkcia
unsigned int htab_hash_function(char *str);

// inicializuje tabuľku, vráti na ňu ukazateľ
// ak sa nepodarilo alokovať vracia NULL
htab_t * htab_init();             

// vloží do tabuľky t prvok s klúčom key a hodnotou val
void htab_insert(htab_t *t, char *key, int val);

// vyhľadá a vráti hodnotu prvku s kľúčom key, 
// pri neúspešom vyhľadaní vráti -1
int htab_find(htab_t *t, char *key);

// vyčistí tabuľku (tabuľka je po vykonaní prázdna)
void htab_clear(htab_t * t);	

// uvoľní tabuľku z pamäte (volá htab_clear)
void htab_free(htab_t * t);		

#endif // __HTABLE_H__
