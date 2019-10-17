#include <stdio.h>
#include <ctype.h> // isalpha, isdigit...
#include <stdlib.h>
#include <string.h>

#define LEX_ERROR 1
#define TOKEN_OK 0

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
#define STATE_NUMBER_FLOAT 11
#define STATE_NUMBER_EXPONENT 12
#define STATE_STRING 13



#define DYNAMIC_STRING_DEFAULT 8 // defaultní velikost pro dynamické pole znaků



typedef enum
{
	// global
	TOKEN_EOF,
	TOKEN_EOL,
	TOKEN_IDENTIFIER,
	TOKEN_KEYWORD,

	// datové typy
	TOKEN_INT,
	TOKEN_FLOAT,
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
	TOKEN_LEFT_BRACKET,
	TOKEN_RIGHT_BRACKET,
	TOKEN_COMMA,
	TOKEN_COLON, // " : "

}TokenTYPE;

typedef enum
{
	KEYWORD_IF,
	KEYWORD_ELSE,
	KEYWORD_RETURN,
	KEYWORD_DEF, // def (definování funkce)
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
}TokenKEYWORD;

typedef struct Token
{
	char* dynamic_value; 
	int integer;
	double decimal; // float
	TokenTYPE type;
	TokenKEYWORD keyword;
}*TokenPTR;


// vstupní soubor s programem
FILE* source_f;
// deklarace funkcí
void setSourceFile(FILE *f);
TokenPTR makeToken(TokenPTR* token);
void updateDynamicString(TokenPTR token);
void IntegerConcatenate(char a, TokenPTR token);
int ExponentConcatenate(int number_Exponent , char b) ;
void freeMemory(TokenPTR token);
int getToken(TokenPTR* token);
