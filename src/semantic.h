/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   semantic.h
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta   <xkucht09@stud.fit.vutbr.cz>
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "stdbool.h"

typedef enum
{
	S,    // < SHIFT
	E,    // = EQUAL
	R,    // > REDUCE
	N     // # ERROR
} Prec_table_sign_enum;



typedef enum
{
	I_PLUS_MINUS,		/// 0 +-
	I_MUL_DIV,			/// 1 */
	I_IDIV,				/// 2 \ /
	I_REL_OP,			/// 3 r (realtion operators) = !> <= < >= >
	I_LEFT_BRACKET,		/// 4 (
	I_RIGHT_BRACKET,	/// 5 )
	I_DATA,				/// 6 i (id, int, double, string)
	I_DOLLAR,			/// 7 $
	I_STOP				/// 8 STOP
} Prec_table_index_enum;


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


typedef struct stacktokenitem
{
	TokenTYPE data_type;
	Prec_table_index_enum token_type;
	struct stacktokenitem *next_token;
} *TStackTokenItem;

typedef struct stacktoken
{
	struct stacktokenitem *top;
} TStackToken;


// function to convert symbol to precedence table index.
Prec_table_index_enum get_prec_table_index(TokenTYPE symbol);


int expression();


int reduce(TStackToken *stack);


int semantic(TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3, Prec_rules_enum rule, TokenTYPE *final_token_type);


Prec_rules_enum test_rule(int count, TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3);



int countTokenStack(TStackToken *stack);


int totalCountTokenStack(TStackToken *stack);


/**
 * Inicializace pomocného stacku pro realizaci "INDENT / DEDENT"
**/
void initTokenStack(TStackToken *stack);

/**
 * Funkce "pushne" hodnotu aktuálního odsazení na stack
**/
void pushTokenStack(TStackToken* stack, TokenTYPE data_type, Prec_table_index_enum token_type);

/**
 * Funkce "popne" aktuální vrchol stacku
**/
bool popTokenStack(TStackToken* stack);

/**
 * Funkce zruší celý stack a korektně uvolní alokovaný prostor
**/
void destroyTokenStack(TStackToken* stack);

/** konec souboru "scanner.h" **/






#endif