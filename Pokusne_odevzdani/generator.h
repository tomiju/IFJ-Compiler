/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   generator.h
 *
 *
 * Datum:    30.11.2019
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

typedef struct Instr{
	int type;				// názov inštrukcie
	tInstrPar* param[3];
} tInstr;

typedef struct Node{
	tInstr instr;
	struct Node* next;
	struct Node* prev;
} tNode;

typedef struct List{
	tNode* first;
	tNode* last;
	tNode* active;
} tList;

// Inicializuje list
void InitList(tList* list);

// Vyprázdni a uvoľní lsit
void DisposeList(tList* list);

// vloží ako prvý
int InsertFirst(tList* list, tInstr instr);

// vloží ako posledný
int InsertLast(tList* list, tInstr instr);

// nastaví aktivitu na prvý
void First(tList* list);

// nastaví aktivitu na posledný
void Last(tList* list);

// nastaví aktivitu na konkrétny prvok
void SetActive(tList* list, tNode* node);

// vráti hodnotu prvého
void CopyFirst(tList* list, tInstr* instr);

// vráti hodnotu posledného
void CopyLast(tList* list, tInstr* intr);

// zmaže prvý
void DeleteFirst(tList* list);

// zmaže posledný
void DeleteLast(tList* list);

// zmaže prvok za aktívnym
void PostDelete(tList* list);

// zmaže prvok pred aktívnym
void PreDelete(tList* list);

// vloží prvok za aktívnym
int PostInsert(tList* list, tInstr instr);

// vloží prvok pred aktívnym
int PreInsert(tList* list, tInstr instr);

// vráti hodnotu
int Copy(tList* list, tInstr* instr);

// aktualizuje hodnotu
void Actualize(tList* list, tInstr instr);

// posunie aktuálny na nasledujúci
void Succ(tList* list);

// posunie aktuálny na predchádzajúci
void Pred(tList* list);

// skontroluje, či má list aktívny prvok
int Active(tList* list);

/************************** STACK **************************/
typedef struct stack{
	tNode* node;
	struct stack* link;
}tStack;

void tPushStack(tStack** stack, tNode* node);

tNode* tPopStack(tStack** stack);

typedef struct stackNum{
	int num;
	struct stackNum* link;
}tStackNum;

void tPushStackNum(tStackNum** stack, int num);

int tPopStackNum(tStackNum** stack);

/*********************** INŠTRUKCIE ************************/

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

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum INSTR_ENUM {
    INSTR(GENERATE_ENUM)
};

void generate_return_variable(tList* list_instr);

void generate_copy_params(tList* list_instr, int count, ...);

// vstavané funkcie
void generate_inputs(tList* list);
void generate_inputf(tList* list);
void generate_inputi(tList* list);
void generate_len(tList* list);
void generate_ord(tList* list);
void generate_substr(tList* list);

/*******************************************************/
/********************  GENEROVANIE  ********************/
/*******************************************************/

// vracia ukazateľ do tabuľky symbolov na konštantu s názvom name
// ak už existuje, nájde ju a vráti. Ak nie, vľoží ju do tabuľky
htab_item_t* make_const(char* name, int type);

// ak ešte neexistuje, vytvorí premennú a vygeneruje pre ňu inštrukciu
htab_item_t* generate_var(tList* list, char* name, int type, int frame);

htab_item_t* make_var(char* name, int type, int frame);

// pridá inštrukciu do listu, predanú cez enum, následuje počet parametrov
// a parametre cez htab_item_t*
void generate_instr(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...);

// volá sa na začiatku generovania
// nageneruje základnú štruktúru a vstavané funkcie
void generator_start(tList* list);

// vráti argument funckie indexovaný od 0
// využíva sa v tele funkcie
htab_item_t* get_param(unsigned idx);

// volá sa na začiatku generovania funkcie
// label je odkaz do tabuľky symbolov, kde sa nachádza funkcia
// vytvorí zarážku, frame a návratovú hodnotu pre funkciu
void generate_func_start(tList* list, htab_item_t* label);

// uloží hodnotu do ktorú funkcia vracia cez return
void generate_save_to_return(tList* list, htab_item_t* value_to_save);

// volá sa na konci funkcie pri dedente
void generate_func_end(tList* list);

// výsledok funkcie uloží do premmenej var
void generate_save_return_value(tList* list, htab_item_t* var);

// pošle parameter do funkcie pred jej zavolaním
void send_param(htab_item_t* par);

// zastaví posielanie parametrov, predá sa funkcia
// vygeneruje sa jej volanie zo zadanými parametrami
void func_call(tList* list, htab_item_t* func);

// nageneruje volanie funkcie
// vytvorí rámec a na ňom predá parametre do funkcie, zavolá funckiu
// list, odkaz na funkciu v hashtable, počet argumentov
// a následne odkazy do hash table na jednotlivé argumenty
void generate_func_call(tList* list, htab_item_t* label, unsigned count, ...);

// vráti premennú pre podmienku aktuálne spracovávaného cyklu
htab_item_t* get_while_cond();

// if there is declaration inside if or else, call this function
// to generate var item before them
void generate_before_if(tList* list, htab_item_t* item);

// if there is declaration inside while, call this function
// to generate var item before while
void generate_before_whiles(tList* list, htab_item_t* item);

// na začiatku while, po ňom sa vytvorí podmienka a telo cyklu
void generate_while_start(tList* list);

// pri každom returne
void generate_return(tList* list);

// na konci cyklu
void generate_while_end(tList* list);

// nageneruje kontrolu podmienky a skoky
void generate_condition_check(tList* list, htab_item_t* podmienka, bool isWhile);

// na začiatku
// vytovrí kostru pre if-else
void start_if_else(tList* list);

// volá sa, keď sa ide generovať telo if-u
void generate_if(tList* list);

// volá sa, keď sa ide generovať telo else-u
void generate_else(tList* list);

// volá sa na konci po vygenerovaní else
void end_if_else(tList* list);

// volá sa na záver programu
// vypíše všetky nagenerované inštrukcie na výstup
void printInstructions(tList* list);

/*******************************************************/
/************************ END **************************/
/*******************************************************/

#endif // __GENERATOR_H__
