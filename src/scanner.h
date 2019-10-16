#include <stdio.h>
#include <ctype.h> // isalpha, isdigit...
#include <stdlib.h>

#define LEX_ERROR 1
#define TOKEN_OK 0

#define STATE_START 0
#define STATE_COMMENTARY 1
#define STATE_BLOCK_COMMENTARY 2
#define STATE_BLOCK_COMMENTARY_END 3


#define DYNAMIC_STRING_DEFAULT 8 // defaultní velikost pro dynamické pole znaků



typedef enum
{
	TOKEN_EOF,
	TOKEN_EOL,
	TOKEN_IDENTIFIER,
	TOKEN_KEYWORD,

	TOKEN_INT,
	TOKEN_FLOAT,
	TOKEN_STRING,
	TOKEN_NONE,

	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_IDIV, // celočíselné dělení

	TOKEN_LESS_THAN,
	TOKEN_MORE_THAN,
	TOKEN_LESS_THAN_OR_EQUAL,
	TOKEN_MORE_THAN_OR_EQUALM,
	TOKEN_NOT_EQUAL, // !=
	TOKEN_EQUAL, // == 
	TOKEN_ASSIGN, // =

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
	double decimal;
	int error;
	TokenTYPE type;
	TokenKEYWORD keyword;
}*TokenPTR;

FILE* source_f;
void setSourceFile(FILE *f);
TokenPTR makeToken(TokenPTR* token);
void updateDynamicString(TokenPTR token);
void freeMemory(TokenPTR token);
int getToken(TokenPTR* token);
