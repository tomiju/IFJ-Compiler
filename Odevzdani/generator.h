/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   generator.h
 *
 *
 * Datum:    10.12.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include "symtable.h"

/********************* LIST INŠTRUKCIÍ *********************/
// parametre operandov
typedef struct InstrPar{
	int ival;
	double dval;
	char* sval;
	char* key;
	int type;
	int frame;
	bool isLabel;
	bool isConst;
} tInstrPar;

// inštrukcia s operandmi
typedef struct Instr{
	int type;				// názov inštrukcie
	tInstrPar* param[3];
} tInstr;

// položka zoznamu
typedef struct Node{
	tInstr instr;
	struct Node* next;
	struct Node* prev;
} tNode;

// zoznam inštrukcií
typedef struct List{
	tNode* first;
	tNode* last;
	tNode* active;
} tList;

// Inicializuje list
void InitList(tList* list);

// Vyprázdni a uvoľní list
void DisposeList(tList* list);

// vloží ako posledný
int InsertLast(tList* list, tInstr instr);

// nastaví aktivitu na prvý
void First(tList* list);

// nastaví aktivitu na posledný
void Last(tList* list);

// nastaví aktivitu na konkrétny prvok
void SetActive(tList* list, tNode* node);

// vloží prvok za aktívny
// vráti INTERNAL_ERROR pri chybe
int PostInsert(tList* list, tInstr instr);

// vráti aktuálnu inštrukciu
tInstr* Copy(tList* list, tInstr* instr);

// posunie aktuálny na nasledujúci
void Succ(tList* list);

// skontroluje, či má list aktívny prvok
// vracia 1 ak áno, ináč 0
int Active(tList* list);

/************************** STACK **************************/
// stack ukazateľov na inštrukcie
typedef struct stack{
	tNode* node;
	struct stack* link;
}tStack;

// vloží na vrchol stacku
// ak sa nepodarila alokácia vracia INTERNAL_ERROR, ináč 0
int tPushStack(tStack** stack, tNode* node);

// vráti hodnotu z vrchou stacku, neodstráni ju zo stacku
tNode* tTopStack(tStack* stack);

// vráti hodnotu z vrchou stacku a zároveň ju odstráni
tNode* tPopStack(tStack** stack);

// stack indexov
typedef struct stackNum{
	int num;
	struct stackNum* link;
}tStackNum;

// vloží na vrchol stacku
// ak sa nepodarila alokácia vracia INTERNAL_ERROR, ináč 0
int tPushStackNum(tStackNum** stack, int num);

// vráti hodnotu z vrchou stacku, neodstráni ju zo stacku
int tTopStackNum(tStackNum* stack);

// vráti hodnotu z vrchou stacku a zároveň ju odstráni
int tPopStackNum(tStackNum** stack);

/*********************** INŠTRUKCIE ************************/
// Zoznam inštrukcií
#define INSTR(x) \
        x(MOVE)			/* <var> <symb> */				\
        x(CREATEFRAME)  								\
        x(PUSHFRAME)   									\
        x(POPFRAME)  									\
        x(DEFVAR)  		/* <var> */						\
        x(CALL)  		/* <label> */					\
        x(RETURN) 										\
        x(PUSHS)  		/* <symb> */					\
        x(POPS)  		/* <var> */						\
        x(CLEARS)  										\
        x(ADD)  		/* <var> <symb1> <symb2> */		\
        x(SUB)  		/* <var> <symb1> <symb2> */		\
        x(MUL)  		/* <var> <symb1> <symb2> */		\
        x(DIV)  		/* <var> <symb1> <symb2> */		\
        x(IDIV)  		/* <var> <symb1> <symb2> */		\
        x(ADDS)  										\
        x(SUBS)  										\
        x(MULS)  										\
        x(DIVS)  										\
        x(IDIVS)  										\
        x(LT)  			/* <var> <symb1> <symb2> */		\
        x(GT)  			/* <var> <symb1> <symb2> */		\
        x(EQ)	  		/* <var> <symb1> <symb2> */		\
        x(LTS)  										\
        x(GTS)  										\
        x(EQS)  										\
        x(AND)  		/* <var> <symb1> <symb2> */		\
        x(OR)	  		/* <var> <symb1> <symb2> */		\
        x(NOT)  		/* <var> <symb1> */				\
        x(ADNS)  										\
        x(ORS)  										\
        x(NOTS)  										\
        x(INT2FLOAT)	/* <var> <symb> */				\
		x(FLOAT2INT)	/* <var> <symb> */				\
		x(INT2CHAR)		/* <var> <symb> */				\
		x(STRI2INT)		/* <var> <symb1> <symb2> */		\
		x(INT2FLOATS)									\
		x(FLOAT2INTS)									\
		x(INT2CHARS)									\
		x(STRI2INTS)									\
		x(READ)			/* <var> <type> */				\
		x(WRITE)		/* <symb> */ 					\
		x(CONCAT)		/* <var> <symb1> <symb2> */		\
		x(STRLEN)		/* <var> <symb> */				\
		x(GETCHAR)		/* <var> <symb1> <symb2> */		\
		x(SETCHAR)		/* <var> <symb1> <symb2> */		\
		x(TYPE)			/* <var> <symb> */				\
		x(LABEL)		/* <label> */					\
		x(JUMP)			/* <label> */					\
		x(JUMPIFEQ)		/* <label> <symb1> <symb2> */	\
		x(JUMPIFNEQ)	/* <label> <symb1> <symb2> */	\
		x(JUMPIFEQS)									\
		x(JUMPIFNEQS)									\
		x(EXIT)			/* <symb> */					\
		x(BREAK)										\
		x(DPRINT)		/* <symb> */					\

/************************ MAKRÁ *************************/
// vygeneruje enum so všetkými inštrukciami
#define GENERATE_ENUM(ENUM) ENUM,
// vygeneruje zoznam stringov so všetkými inštrukciami
#define GENERATE_STRING(STRING) #STRING,

// vytvorí enum inštrukcií
enum INSTR_ENUM {
    INSTR(GENERATE_ENUM)
};

/************  GENEROVANIE VSTAVANÝCH FUNKCIÍ  ************/
// vstavané funkcie
// pri chybe vrátia INTERNAL_ERROR, ináč 0
int generate_inputs(tList* list);
int generate_inputf(tList* list);
int generate_inputi(tList* list);
int generate_len(tList* list);
int generate_ord(tList* list);
int generate_substr(tList* list);
int generate_print(tList* list);

/*******************************************************/
/********************  GENEROVANIE  ********************/
/*******************************************************/

// vracia ukazateľ do tabuľky symbolov na konštantu s názvom name
// ak už existuje, nájde ju a vráti. Ak nie, vľoží ju do tabuľky
// pri neúspechu vracia NULL
htab_item_t* make_const(char* name, int type);

// ak ešte neexistuje, vytvorí premennú a vygeneruje pre ňu inštrukciu
// pri neúspechu vracia NULL
htab_item_t* generate_var(tList* list, char* name, int type, int frame);

// ak ešte neexistuje, vytvorí premennú a vráti ukazateľ do tabuľky symbolov
// pri neúspechu vracia NULL
htab_item_t* make_var(char* name, int type, int frame);

// pridá inštrukciu do listu, predanú cez enum, následuje počet parametrov
// a parametre cez htab_item_t*
// nerobí kontrolu typov
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int generate_instr_no(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...);

// pridá inštrukciu do listu, predanú cez enum, následuje počet parametrov
// a parametre cez htab_item_t*
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int generate_instr(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...);

// volá sa na začiatku generovania
// nageneruje základnú štruktúru a vstavané funkcie
// pri chybe vráti INTERNAL_ERROR, ináč 0
int generator_start(tList* list);

// vráti argument funckie indexovaný od 0
// využíva sa v tele funkcie
htab_item_t* get_param(unsigned idx);

// volá sa na začiatku generovania funkcie
// label je odkaz do tabuľky symbolov, kde sa nachádza funkcia
// vytvorí zarážku, frame a návratovú hodnotu pre funkciu
// pri chybe vráti INTERNAL_ERROR, ináč 0
int generate_func_start(tList* list, htab_item_t* label);

// uloží hodnotu do ktorú funkcia vracia cez return
// pri chybe vráti INTERNAL_ERROR, ináč 0
int generate_save_to_return(tList* list, htab_item_t* value_to_save);

// volá sa na konci funkcie pri dedente
int generate_func_end(tList* list);

// výsledok funkcie uloží do premmenej var
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int generate_save_return_value(tList* list, htab_item_t* var);

// pošle parameter do funkcie pred jej zavolaním
void send_param(htab_item_t* par);

// zastaví posielanie parametrov, predá sa funkcia
// vygeneruje sa jej volanie zo zadanými parametrami
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int func_call(tList* list, htab_item_t* func);

// nageneruje volanie funkcie
// vytvorí rámec a na ňom predá parametre do funkcie, zavolá funckiu
// list, odkaz na funkciu v hashtable, počet argumentov
// a následne odkazy do hash table na jednotlivé argumenty
int generate_func_call(tList* list, htab_item_t* label, unsigned count, ...);

// vráti premennú pre podmienku aktuálne spracovávaného cyklu
htab_item_t* get_while_cond();

// if there is declaration inside if or else, call this function
// to generate var item before them
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int generate_before_if(tList* list, htab_item_t* item);

// if there is declaration inside while, call this function
// to generate var item before while
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int generate_before_whiles(tList* list, htab_item_t* item);

// na začiatku while, po ňom sa vytvorí podmienka a telo cyklu
// pri neúspechu vracia INTERNAL_ERROR, ináč 0
int generate_while_start(tList* list);

// pri každom returne
// pri chybe vráti INTERNAL_ERROR, ináč 0
int generate_return(tList* list);

// na konci cyklu
void generate_while_end(tList* list);

// nageneruje kontrolu podmienky a skoky
// pri chybe vracia INTERNAL_ERROR, ináč 0
int generate_condition_check(tList* list, htab_item_t* podmienka, bool isWhile);

// na začiatku
// vytovrí kostru pre if-else
// pri chybe vracia INTERNAL_ERROR, ináč 0
int start_if_else(tList* list);

// volá sa, keď sa ide generovať telo if-u
void generate_if(tList* list);

// volá sa, keď sa ide generovať telo else-u
void generate_else(tList* list);

// volá sa na konci po vygenerovaní else
void end_if_else(tList* list);

// volá sa na záver programu
// vypíše všetky nagenerované inštrukcie na výstup
// pri chybe vracia INTERNAL_ERROR, ináč 0
int printInstructions(tList* list);

/*******************************************************/
/************************ END **************************/
/*******************************************************/

#endif // __GENERATOR_H__
