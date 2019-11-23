#ifndef __HTABLE_H__
#define __HTABLE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// veľkosť tabuľky
#define SIZE 100

// htab_item "frame" constants
#define GF 0
#define LF 1
#define TF 2

// htab_item "type" constants
#define INT 0
#define FLOAT 1
#define STRING 2
#define BOOL 3
#define NIL 4	
#define UNKNOWN 5
#define FUNC 6
#define TYPE_NAME 7

struct htab{
	struct htab_item* ptr[SIZE]; 
};

// položky
typedef struct htab_item {
	int value;
	char* key;
	int type;
	int frame;	
	bool isLabel;
	bool isConst;
	struct htab_item *next;		// ďalšia položka (synonymum)
	int reviewed;
	bool defined;			
	struct htab* local_vars;	// symtable pre lokálne premenné
} htab_item_t;

// tabuľka
typedef struct htab htab_t;

// vloží vstavané funkcie do hash table
void htab_insert_default_functions(htab_t *htab);

// hashovacia funkcia, vracia index do tabuľky
unsigned int htab_hash_function(char *str);

// inicializuje tabuľku predanú ukazovateľom
// pri úspechu vracia 0, pri chybe INTERNAL_ERROR
int htab_init(htab_t **htab);           

// vyhľadá a vráti ukazateľ na konkrétny prvok
// ak sa nenašiel, vráti NULL
htab_item_t* htab_find(htab_t *t, char *key);

// ak sa v tabuľke prvok s kľúčom key nenachádza
// vloží do tabuľky t prvok s klúčom key, hodnotou val 
// typom, príznakom isConst a isLabel a defined
// ak sa nachádza, zmení jeho hodnoty
// pri úspechu vracia 0, pri chybe INTERNAL_ERROR
int htab_insert(htab_t *t, char *key, int val, int type, int frame, bool isConst, bool isLabel, bool defined);

// vyčistí tabuľku (tabuľka je po vykonaní prázdna)
void htab_clear(htab_t * t);	

// uvoľní tabuľku z pamäte (volá htab_clear)
void htab_free(htab_t * t);		

#endif // __HTABLE_H__
