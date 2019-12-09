/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   expression.h
 *
 *
 * Datum:    30.11.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta   <xkucht09@stud.fit.vutbr.cz>
 */

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "stdbool.h"
#include "symtable.h"

/**
 * Operácie v precedenčnej tabuľke
 */

typedef enum
{
	S,    // < SHIFT
	E,    // = EQUAL
	R,    // > REDUCE
	N     // # ERROR
} Prec_table_sign_enum;

/**
 * Prvky v precedenčnej tabuľke
 */

typedef enum
{
	I_PLUS_MINUS,		/// 0 +-
	I_MUL_DIV,			/// 1 */
	I_IDIV,				/// 2 //
	I_REL_OP,			/// 3 r (relačné operátory) = !> <= < >= >
	I_LEFT_BRACKET,		/// 4 (
	I_RIGHT_BRACKET,	/// 5 )
	I_DATA,				/// 6 i (id, int, double, string)
	I_DOLLAR,			/// 7 $
	I_STOP				/// 8 STOP
} Prec_table_index_enum;

/**
 * Pravidlá na redukovanie
 */

typedef enum
{
	NT_EQ_NT,		/// E -> E = E
	NT_NEQ_NT,		/// E -> E <> E
	NT_LEQ_NT,		/// E -> E <= E
	NT_LTN_NT,		/// E -> E < E
	NT_MEQ_NT,		/// E -> E => E
	NT_MTN_NT,		/// E -> E > E
	NT_PLUS_NT,		/// E -> E + E
	NT_MINUS_NT,	/// E -> E - E
	NT_IDIV_NT,		/// E -> E \ E
	NT_MUL_NT,		/// E -> E * E
	NT_DIV_NT,		/// E -> E / E
	LBR_NT_RBR,		/// E -> (E)
	OPERAND,		/// E -> i
	NOT_A_RULE		/// rule doesn't exist
} Prec_rules_enum;

/**
 * Štruktúra predstavujúca prvok v zásobníku
 */

typedef struct stacktokenitem
{
	TokenTYPE data_type;
	Prec_table_index_enum token_type;
	htab_item_t *table_symbol;
	struct stacktokenitem *next_token;
} *TStackTokenItem;

/**
 * Zásobník
 */

typedef struct stacktoken
{
	struct stacktokenitem *top;
} TStackToken;


/**
 * Funkcia vracia číslo prvku tabuľky na základe prijatého tokenu
 * symbol: typ tokenu
 */

Prec_table_index_enum get_prec_table_index(TokenTYPE symbol);

/**
 * Hlavná funkcia na vyhodnotenie výrazu
 * htab_symbol: ukazateľ do tabuľky symbolov
 */

int expression(htab_item_t* htab_symbol);

/**
 * Funkcia, ktorá vkladá token na zásobník
 * stack: unakateľ na zásobník
 */

int shift(TStackToken *stack);

/**
 * Funkcia na redukciu častí výrazu
 * stack: ukazateľ na zásobník
 */

int reduce(TStackToken *stack);

/**
 * Funkcia na sémantické kontroly vo výrazoch
 * op1, op2, op3: prvky na vrchole zásobnníka
 * htab_symbol: ukazateľ do tabuľky symbolov
 * rule: pravidlo, podľa ktorého sa má vykonať kontrola
 * final_token_type: typ konečného podvýrazu
 */

int semantic(TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3, htab_item_t* htab_symbol, Prec_rules_enum rule, TokenTYPE *final_token_type);

/**
 * Funkcia na kontrolu operátorov a operandy
 * count: počet operátorov- 1 alebo 3
 * op1, op2, op3: prvky na vrchole zásobníka
 */

Prec_rules_enum test_rule(int count, TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3);

/**
 * Funkcia vracia počet prvkov pred symbolom STOP
 * stack: ukazateľ na zásobník
 */

int countTokenStack(TStackToken *stack);

/**
 * Funkcia vracia počet prvkov pred symbolom DOLLAR
 * stack: ukazateľ na zásobník
 */

int totalCountTokenStack(TStackToken *stack);

/**
 * Inicializácia zásobníku na výrazy
 * stack: ukazateľ na zásobník
 */

void initTokenStack(TStackToken *stack);

/**
 * Funkcia "pushne" token na zásobník
 * stack: ukazateľ na zásobník
 * DataType, TokenType: informácie o tokene
 * htab_symbol: ukazateľ do tabuľky symbolov
 */

int pushTokenStack(TStackToken* stack, TokenTYPE DataType, Prec_table_index_enum TokenType, htab_item_t *htab_symbol);

/**
 * Funkcia "popne" prvok z vrcholu zásobníku
 * stack: ukazateľ na zásobník
 */

bool popTokenStack(TStackToken* stack);

/**
 * funkcia vyprázdni zásobník a zruší ho
 * stack: ukazateľ na zásobník
 */

void destroyTokenStack(TStackToken* stack);

/**
 * Funkcia na generovanie mien pomocných premenných
 */

char* get_name();


/** koniec súboru "expression.h" **/

#endif
