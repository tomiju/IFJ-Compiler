/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   scanner.h
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 * 			 Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#include <stdio.h>
#include <ctype.h> // isalpha, isdigit...
#include <stdlib.h>
#include <string.h>

#define LEX_ERROR 1
#define TOKEN_OK 0
#define TRUE 1
#define FALSE 0

/**
 * Stavy
**/
#define STATE_START 0
#define STATE_COMMENTARY 1
#define STATE_BLOCK_COMMENTARY 2
#define STATE_BLOCK_COMMENTARY_END 3
#define STATE_LESS_THAN 4
#define STATE_MORE_THAN 5
#define STATE_ASSIGN 6
#define STATE_EQUAL 7
#define STATE_DIV 8
#define STATE_NOT_EQUAL 9
#define STATE_NUMBER_INT 10
#define STATE_NUMBER_DOUBLE 11
#define STATE_NUMBER_EXPONENT 12
#define STATE_STRING 13
#define STATE_IDENTIFIER 14
#define STATE_INDENT 15
#define STATE_DEDENT 16
#define STATE_ESCAPE_SEQUENCE 17

#define DYNAMIC_STRING_DEFAULT 8 // defaultní velikost pro dynamické pole znaků

/**
 * Pomocný enum s ID typů tokenů
**/
typedef enum
{
	TOKEN_DEFAULT,
	// global
	TOKEN_EOF,
	TOKEN_EOL,
	TOKEN_IDENTIFIER,
	TOKEN_KEYWORD,

	// datové typy
	TOKEN_INT,
	TOKEN_DOUBLE,
	TOKEN_STRING,
	TOKEN_NONE,

	// operátory
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_IDIV, // celočíselné dělení

	TOKEN_LESS_THAN, //13
	TOKEN_MORE_THAN,
	TOKEN_LESS_THAN_OR_EQUAL, //15
	TOKEN_MORE_THAN_OR_EQUAL,
	TOKEN_NOT_EQUAL, // !=
	TOKEN_EQUAL, // == 
	TOKEN_ASSIGN, // =

	// ostatní
	TOKEN_INDENT,
	TOKEN_DEDENT,
	TOKEN_NO_INDENT_OR_DEDENT, // stejná úroveň zanoření
	TOKEN_LEFT_BRACKET,
	TOKEN_RIGHT_BRACKET,
	TOKEN_COMMA,
	TOKEN_COLON, // " : "

	// keywords
	KEYWORD_DEFAULT,
	KEYWORD_IF,
	KEYWORD_ELSE,
	KEYWORD_RETURN,
	KEYWORD_DEF, // def (definování funkce)
	KEYWORD_NONE,
	KEYWORD_WHILE,
	KEYWORD_INPUTS,
	KEYWORD_INPUTI,
	KEYWORD_INPUTF,
	KEYWORD_PRINT,
	KEYWORD_LEN,
	KEYWORD_SUBSTR,
	KEYWORD_ORD,
	KEYWORD_CHR,
	KEYWORD_PASS,

}TokenTYPE;

/**
 * Struktura tokenu
**/
typedef struct Token // struktura tokenu
{
	char* dynamic_value; // reálný obsah tokenu
	int size; // délka uloženého stringu
	int allocated_size; // alokovaná velikost
	double number_value; // velikost bez zkráceného zápisu (e/E)
	TokenTYPE type; // informace o typu tokenu
}*TokenPTR;

/**
 * Pomocná struktura pro stack "INDENT" a "DEDENT"
**/
typedef struct indentStack // struktura pro stack (pevná velikost -> předělat na dynamickou)
{
	int value; // hodnota na vrcholu stacku
	int level; // úroveň zanoření
	struct indentStack* link; // odkaz na další položku na stacku
}*iStack;

// vstupní soubor s programem

FILE* source_f;

/*********************
 * Deklarace funkcí
**********************/

/**
 * Funkce nastaví zdrojový soubor
**/
void setSourceFile(FILE *f);

/**
 * Funkce vytvoří nový token a uloží do pointeru "token"
**/
TokenPTR makeToken(TokenPTR* token);

/**
 * Realizace konečného automatu celého scanneru
**/
int getToken(TokenPTR* token, iStack* indent_stack);

/**
 * Funkce vrací token stejně jako funkce getToken(),
 * ale neposouvá se v souboru
**/
int preloadToken(TokenPTR* token, iStack* indent_stack);

/**
 * Funkce uvolní všechny alokované prostředky
**/
void freeMemory(TokenPTR token, iStack* indent_stack);

/**
 * Funkce uloží další znak dynamicky do struktury token
 * v případě, že není dostatek alokovaného prostoru alokuje další
**/
int updateDynamicString(char currentChar, TokenPTR token);

/**
 * Funkce pro převod číselné hodnoty z "char" do int
**/
void computeNumberWithExponent(TokenPTR token);

/**
 * Funkce, která zkontroluje, zda identifikátor není klíčovým slovem
**/
int checkKeyword(TokenPTR token);

/**
 * Funkce pro výpis obsahu tokenu
**/
void debugToken(TokenPTR* token, iStack* indent_stack);

/**
 * Inicializace pomocného stacku pro realizaci "INDENT / DEDENT"
**/
iStack initStack();

/**
 * Funkce "pushne" hodnotu aktuálního odsazení na stack
**/
void pushStack(iStack* indent_stack, int current_indent_value);

/**
 * Funkce "popne" aktuální vrchol stacku
**/
void popStack(iStack* indent_stack);

/**
 * Funkce zruší celý stack a korektně uvolní alokovaný prostor
**/
void destroyStack(iStack* indent_stack);

/** konec souboru "scanner.h" **/