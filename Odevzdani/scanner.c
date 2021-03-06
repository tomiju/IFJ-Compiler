/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   scanner.c
 *
 *
 * Datum:    10.12.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"

static int FirstToken = TRUE; // rozhodnutí, zda se bude generovat indent/dedent nebo ne
static int currentIndent_static; // pomocná proměnná, která si pamatuje aktuální zanoření z minulého volání
static char previousChar_static; // pro \n v řetězci

/**
 * Pomocné funkce pro práci se stackem odsazení
**/
iStack initStack()
{
	iStack newStack = (iStack) malloc(sizeof(struct indentStack));

	if (newStack == NULL)
	{
		return NULL;
	}

	newStack->value = 0;
	newStack->level = 0;
	newStack->link = newStack;

	return newStack;
}

void pushStack(iStack* indent_stack, int currentIndent)
{
	iStack temp = (iStack) malloc(sizeof(struct indentStack));

	if (temp == NULL)
	{
		return;
	}

	temp->value = currentIndent;
	temp->level += (*indent_stack)->level + 1;
	temp->link = *indent_stack;

	*indent_stack = temp;
}

void popStack(iStack* indent_stack)
{
	iStack temp = (iStack) malloc(sizeof(struct indentStack));

	if (indent_stack == NULL || (*indent_stack)->value == 0 || temp == NULL)
	{
		return;
	}

	temp = *indent_stack;
	*indent_stack = (*indent_stack)->link;

	free(temp);
}

void destroyStack(iStack* indent_stack)
{
	iStack temp = (iStack) malloc(sizeof(struct indentStack));

    temp = *indent_stack;

    while (temp->value != 0)
    {
        popStack(indent_stack);

        temp = temp->link;
    }
}

/**
 * Funkce pro práci s tokeny
**/
TokenPTR makeToken(TokenPTR* token) // vytvoří nový token a alokuje paměť
{
	TokenPTR newToken = (TokenPTR) malloc(sizeof(struct Token));

	if (newToken == NULL)
	{
		return NULL;
	}

	*token = newToken;

	newToken->dynamic_value = (char*) malloc(DYNAMIC_STRING_DEFAULT);

	if (newToken->dynamic_value == NULL)
	{
		free(newToken);
		return NULL;
	}

	newToken->number_value = 0.0;
	newToken->size = 0;
	newToken->allocated_size = DYNAMIC_STRING_DEFAULT;
	newToken->dynamic_value[newToken->size] = '\0';
	newToken->type = TOKEN_DEFAULT;

	return newToken;
}

int updateDynamicString(char currentChar, TokenPTR token)
{
	if (token->size + 1 >= token->allocated_size)
	{
		int realloc_size = token->allocated_size + DYNAMIC_STRING_DEFAULT;

		token->dynamic_value = (char*) realloc(token->dynamic_value, realloc_size);
		if (token->dynamic_value == NULL)
		{
			return 1;
		}
		token->allocated_size = realloc_size;
	}

	token->dynamic_value[token->size++] = currentChar;
	token->dynamic_value[token->size] = '\0';

	return 0;
}

void ProcessCharToNumber(TokenPTR token)
{
	token->number_value = (atof(token->dynamic_value));
}

void freeMemory(TokenPTR token, iStack* indent_stack)
{
	free(token->dynamic_value);
	free(token);
	destroyStack(indent_stack);
}

int checkKeyword(TokenPTR token)
{
	int pom;

	if ((pom = strcmp("if", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_IF;
	 	return 0;
	}
	if ((pom = strcmp("else", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_ELSE;
	 	return 0;
	}
	if ((pom = strcmp("return", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_RETURN;
	 	return 0;
	}
	if ((pom = strcmp("def", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_DEF;
	 	return 0;
	}
	if ((pom = strcmp("while", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_WHILE;
	 	return 0;
	}
	if ((pom = strcmp("pass", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_PASS;
	 	return 0;
	}
	if ((pom = strcmp("None", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_NONE;
	 	return 0;
	}

	return -1;
}

void debugToken(TokenPTR* token, iStack* indent_stack)
{
	const char* typeNames[] = {"TOKEN_DEFAULT","TOKEN_EOF","TOKEN_EOL","TOKEN_IDENTIFIER","TOKEN_KEYWORD","TOKEN_INT","TOKEN_DOUBLE","TOKEN_STRING","TOKEN_NONE","TOKEN_PLUS","TOKEN_MINUS","TOKEN_MUL","TOKEN_DIV","TOKEN_IDIV","TOKEN_LESS_THAN","TOKEN_MORE_THAN","TOKEN_LESS_THAN_OR_EQUAL","TOKEN_MORE_THAN_OR_EQUAL","TOKEN_NOT_EQUAL","TOKEN_EQUAL","TOKEN_ASSIGN","TOKEN_INDENT","TOKEN_DEDENT","TOKEN_NO_INDENT_OR_DEDENT","TOKEN_LEFT_BRACKET","TOKEN_RIGHT_BRACKET","TOKEN_COMMA","TOKEN_COLON","KEYWORD_DEFAULT","KEYWORD_IF","KEYWORD_ELSE","KEYWORD_RETURN","KEYWORD_DEF","KEYWORD_NONE","KEYWORD_WHILE","KEYWORD_INPUTS","KEYWORD_INPUTI","KEYWORD_INPUTF","KEYWORD_PRINT","KEYWORD_LEN","KEYWORD_SUBSTR","KEYWORD_ORD","KEYWORD_CHR","KEYWORD_PASS"};

	printf("\n");
    printf("value: --> %s <--\n", (*token)->dynamic_value );
    printf("type: %s\n",typeNames[(*token)->type] );
    printf("indent stack top: %d\n", (*indent_stack)->value );
    printf("indent stack level: %d\n", (*indent_stack)->level );
}

/**
 * Hlavní funkce pro vytváření finálního token (FSM = final state machine)
**/
int getToken(TokenPTR* token, iStack* indent_stack)
{
	int state = STATE_START; // stav automatu
	int docStringCounter; // pomocné počítadlo pro začátek / konec dokumentačního řetězce
	int dedentFound; // TRUE = na stacku byla hodnota zanoření (při DEDENT), FALSE = na stacku nebyla -> error
	int currentIndent = 0; // aktuální zanoření pro indent/dedent
	char currentChar, previousChar, firstNumber; // pomocné proměnné
	static int currentIndent_static; // pomocná proměnná, která si pamatuje aktuální zanoření z minulého volání
	static char previousChar_static; // pro \n v řetězci

	TokenPTR newToken = makeToken(token);

	if (newToken == NULL) // pokud byla chyba v alokaci -> error
	{
		return INTERNAL_ERROR;
	}

	while(42)
	{

		currentChar = (char) getc(stdin);

		switch(state)
		{
			case(STATE_START):

				previousChar = currentChar;

				if (currentChar == EOF)
				{
					if ((*indent_stack)->value != 0) // pokud je konec souboru a stack zanoření není prázdny, vygenerují se dedenty
					{
						currentIndent_static = 0;
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_EOF;
					return TOKEN_OK;
				}
				else if (currentChar == '\\')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE) // pokud je aktuální zanoření jiné, než to na vrcholu stacku, generuje se dedent
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}
					state = STATE_ESCAPE_SEQUENCE;
					FirstToken = FALSE;
					break;
				}
				if (currentChar == '\n') // pro indent / dedent... nebo EOL - podle proměnné "FirstInLine"
				{
					if (FirstToken == FALSE) // pokud je nový řádek a už byl nějaký token, tak se vygeneruje EOL, jinak se nový řádek ignoruje
					{
						newToken->type = TOKEN_EOL;
						currentIndent_static = 0;
						FirstToken = TRUE;
						return TOKEN_OK;
					}

					FirstToken = TRUE;
					break;
				}
				else if (currentChar == '\'' && previousChar != '\\')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_STRING;
					FirstToken = FALSE;
					break;
				}
				else if (currentChar == '#')
				{
					state = STATE_COMMENTARY;
					break;
				}
				else if (currentChar == '\"' && previousChar != '\\')
				{
					if (currentIndent_static != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					FirstToken = FALSE;
					state = STATE_DOC_STRING;
					docStringCounter = 1;
					break;
				}
				else if (currentChar == '+')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_PLUS;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					return TOKEN_OK;
				}
				else if (currentChar == '-')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_MINUS;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					return TOKEN_OK;
				}
				else if (currentChar == '*')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_MUL;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					return TOKEN_OK;
				}
				else if (currentChar == '<')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_LESS_THAN;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '>')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_MORE_THAN;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '!')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_NOT_EQUAL;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '=')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_ASSIGN;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '/')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_DIV;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '(')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_LEFT_BRACKET;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;

					}

					return TOKEN_OK;
				}
				else if (currentChar == ')')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_RIGHT_BRACKET;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					return TOKEN_OK;
				}
				else if (currentChar == ',')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_COMMA;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					return TOKEN_OK;
				}
				else if (currentChar == ':')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					newToken->type = TOKEN_COLON;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					return TOKEN_OK;
				}
				else if (isalpha(currentChar) || currentChar == '_')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_IDENTIFIER;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (isdigit(currentChar))
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, stdin);
						break;
					}

					state = STATE_NUMBER_INT;
					previousChar = currentChar;
					firstNumber = currentChar;
					FirstToken = FALSE;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == ' ' && FirstToken == TRUE) // indent
				{
					state = STATE_INDENT;
					currentIndent++;
					break;
				}
				else if(currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == '\r' || currentChar == '\f') // přeskočí zbytečné mezery (asi spíš ne, ale to je jedno)
				{
					break;
				}
				else
				{
					state = STATE_ERROR;
					break;
				}
				break;

			case(STATE_COMMENTARY):

				if (currentChar == '\n' || currentChar == EOF)
				{
					state = STATE_START;
					ungetc(currentChar, stdin);
					currentIndent = 0;
 				}

				break;

			case(STATE_LESS_THAN):

				if (currentChar != '=')
				{
					ungetc(currentChar, stdin); // tento znak už není součástí tohoto tokenu, tudíž se vrátím zpět a zpracuji ho znova
					newToken->type = TOKEN_LESS_THAN;
					return TOKEN_OK;
				}
				else if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					newToken->type = TOKEN_LESS_THAN_OR_EQUAL;
					return TOKEN_OK;
				}

				break;

			case(STATE_MORE_THAN):

				if (currentChar != '=')
				{
					ungetc(currentChar, stdin); // tento znak už není součástí tohoto tokenu, tudíž se vrátím zpět a zpracuji ho znova
					newToken->type = TOKEN_MORE_THAN;
					return TOKEN_OK;
				}
				else if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					newToken->type = TOKEN_MORE_THAN_OR_EQUAL;
					return TOKEN_OK;
				}

				break;

			case(STATE_NOT_EQUAL):

				if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					newToken->type = TOKEN_NOT_EQUAL;
					return TOKEN_OK;
				}
				else
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}

				break;

			case(STATE_ASSIGN):

				if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					newToken->type = TOKEN_EQUAL;
					return TOKEN_OK;
				}
				else
				{
					ungetc(currentChar, stdin);
					newToken->type = TOKEN_ASSIGN;
					return TOKEN_OK;
				}

				break;

			case(STATE_DIV):

				if (currentChar == '/')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					newToken->type = TOKEN_IDIV;
					return TOKEN_OK;
				}
				else
				{
					ungetc(currentChar, stdin);
					newToken->type = TOKEN_DIV;
					return TOKEN_OK;
				}

				break;

			case(STATE_NUMBER_INT):

				if (isdigit(currentChar))
				{
				  if (isdigit(currentChar) && firstNumber == '0') // ošetření přebytečných nul na začátku čísla
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '.')
				{
					state = STATE_NUMBER_DOUBLE;
					previousChar = currentChar;

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				if (currentChar == 'e' || currentChar == 'E')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					state = STATE_NUMBER_EXPONENT;
					previousChar = currentChar;

					break;
				}
				else if (!isdigit(currentChar)) // když není číslo, tak konec tokenu
				{
					newToken->type = TOKEN_INT;
					ProcessCharToNumber(newToken);
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar == '\f')
				{
					newToken->type = TOKEN_INT;
					ProcessCharToNumber(newToken);
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}

				break;

			case(STATE_NUMBER_DOUBLE):

				if (currentChar == 'e' || currentChar == 'E')
				{
					if (previousChar == '.')
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					state = STATE_NUMBER_EXPONENT;
					previousChar = currentChar;
					break;
				}
				else if (isdigit(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar == '\f')
				{
					if (previousChar == '.') // pokud je prázdná posloupnost za "." error
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					newToken->type = TOKEN_DOUBLE;
					ProcessCharToNumber(newToken);
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}
				else if (!isdigit(currentChar))
				{
					if (previousChar == '.')
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					newToken->type = TOKEN_DOUBLE;
					ProcessCharToNumber(newToken);
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}

				break;

			case(STATE_NUMBER_EXPONENT):

				if ((currentChar == '+' || currentChar == '-') && (previousChar == 'e' || previousChar == 'E')) // volitelný unární znak +,-
				{
					previousChar = 'y';
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (isdigit(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
				}
				else if (isalpha(currentChar))
				{
					if (previousChar == 'e' || previousChar == 'E' || previousChar == '+' || previousChar == '-')
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					newToken->type = TOKEN_DOUBLE;
					ProcessCharToNumber(newToken);
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar == '\f')
				{
					if (previousChar == 'e' || previousChar == 'E' || previousChar == '+' || previousChar == '-')
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					newToken->type = TOKEN_DOUBLE;
					ungetc(currentChar, stdin);
					ProcessCharToNumber(newToken);
					return TOKEN_OK;
				}
				else
				{
					if (previousChar == 'e' || previousChar == 'E' || previousChar == '+' || previousChar == '-')
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					newToken->type = TOKEN_DOUBLE;
					ProcessCharToNumber(newToken);
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}

				break;

			case(STATE_IDENTIFIER):

				if (currentChar == '_')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else if (isalnum(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					break;
				}
				else
				{
					if (!(checkKeyword(newToken))) // kontrola, jestli je indetifikátor klíčové slovo
					{
						ungetc(currentChar, stdin);
						return TOKEN_OK;
					}

					ungetc(currentChar, stdin); // pokud není, tak je to identifikátor
					newToken->type = TOKEN_IDENTIFIER;
					return TOKEN_OK;
				}

				break;

			case(STATE_STRING):

				if (currentChar == 'n' && previousChar_static == '\\') // zpracování escape sekvencí s pomocí statické proměnné
				{
					if(updateDynamicString('\n', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar_static = currentChar;
					break;
				}

				if (currentChar == '\"' && previousChar_static == '\\') // zpracování escape sekvencí s pomocí statické proměnné
				{
					if(updateDynamicString('\"', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar_static = currentChar;
					break;
				}

				if (currentChar == '\'' && previousChar_static == '\\') // zpracování escape sekvencí s pomocí statické proměnné
				{
					if(updateDynamicString('\'', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar_static = currentChar;
					break;
				}

				if (currentChar == 't' && previousChar_static == '\\') // zpracování escape sekvencí s pomocí statické proměnné
				{
					if(updateDynamicString('\t', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar_static = currentChar;
					break;
				}

				if (currentChar == '\\' && previousChar_static == '\\') // zpracování escape sekvencí s pomocí statické proměnné
				{
					if(updateDynamicString('\\', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar_static = 'y'; // kvůli situaci "\\\" atd.
 					break;
				}

				if (currentChar == 'x' && previousChar_static == '\\') // zpracování hexadecimálního tvaru escape sekvence
				{
					currentChar = (char) getc(stdin);
					if (isdigit(currentChar) || (currentChar >= (char)65 && currentChar <= (char)70) || (currentChar >= (char)97 && currentChar <= (char)102))
					{
						previousChar_static = currentChar;
						state = STATE_ESCAPE_SEQUENCE;
						break;
					}
					else
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}

				if (currentChar == '\'' && previousChar_static != '\\')
				{
					newToken->type = TOKEN_STRING;
					return TOKEN_OK;
				}
				else if (currentChar == EOF || currentChar == '\n')
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
				else if (currentChar == '\\')
				{
					previousChar = currentChar;
					previousChar_static = currentChar;
					break;
				}
				else
				{
					if (previousChar_static == '\\')
				 	{
						if(updateDynamicString('\\', newToken))
						{
							freeMemory(newToken, indent_stack);
							return INTERNAL_ERROR;
						}
						if(updateDynamicString(currentChar, newToken))
						{
							freeMemory(newToken, indent_stack);
							return INTERNAL_ERROR;
						}

						previousChar = currentChar;
 						previousChar_static = currentChar;
					 	break;
					}
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar = currentChar;
					previousChar_static = currentChar;
					break;
				}

				previousChar = currentChar;
				previousChar_static = currentChar;
				break;

			case(STATE_ESCAPE_SEQUENCE):

				if (isdigit(currentChar) || (currentChar >= (char)65 && currentChar <= (char)70) || (currentChar >= (char)97 && currentChar <= (char)102))
				{
					char tmp[2];
					tmp[0] = previousChar_static;
					tmp[1] = currentChar;

					int tmpToChar = (int)strtol(tmp, NULL, 16); // uložím si ASCII hodnotu znaku z hexadecimálního tvaru

					if(updateDynamicString((char)tmpToChar, newToken)) // uložím tuto hodnotu jako char
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					state = STATE_STRING; // a pokračování do stavu STRING
					break;
				}
				else
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}

			case(STATE_INDENT):

				if (currentChar == ' ')
				{
					currentIndent++;
					currentIndent_static = currentIndent;
					break;
				}

				if ((currentChar == '\n' || currentChar == '#' || currentChar == EOF) && FirstToken == TRUE) // v tomto případě negeneruji indent
				{
					currentIndent = 0;
					currentIndent_static = 0;
					ungetc(currentChar, stdin);
					state = STATE_START;
					break;
				}
				else
				{
					if ((*indent_stack)->value == currentIndent) // na stacku je stejná hodnota, jako aktuální - nic negeneruji
					{
						state = STATE_START;
						ungetc(currentChar, stdin);
						break;
					}
					else if ((*indent_stack)->value > currentIndent) // na stacku je větší hodnota než aktuální - dedent
					{
						dedentFound = FALSE;

						if ((*indent_stack)->value != currentIndent)
						{
							if ((*indent_stack)->level == 1 && currentIndent != 0)
							{
								freeMemory(newToken, indent_stack);
								return LEX_ERROR;
							}

							popStack(indent_stack);
							newToken->type = TOKEN_DEDENT;
							ungetc(currentChar, stdin);
							currentIndent_static = currentIndent;

							if ((*indent_stack)->value == currentIndent) // když najdu hodnotu aktuálního zanoření ve stacku, tak je to ok
							{
								FirstToken = FALSE;
								dedentFound = TRUE;
							}

							if (dedentFound == FALSE && (*indent_stack)->value != currentIndent && (*indent_stack)->level == 1 && currentIndent != 0) // nenašel jsem, není to ok
							{
								freeMemory(newToken, indent_stack);
								return LEX_ERROR;
							}

							return TOKEN_OK;
						}

						if ((*indent_stack)->value == currentIndent) // našel jsem aktuální hodnotu zanoření ve stacku
						{
							dedentFound = TRUE;
							currentIndent_static = 0;
						}

						if (dedentFound == FALSE) // nenašel - error
						{
							freeMemory(newToken, indent_stack);
							return LEX_ERROR;
						}
						else
						{
							newToken->type = TOKEN_DEDENT; // generuje se token
							ungetc(currentChar, stdin);
							FirstToken = FALSE;
							return TOKEN_OK;
						}
					}
					else
					{
						pushStack(indent_stack, currentIndent); // pokud to není dedent, je to indent
						ungetc(currentChar, stdin);
						newToken->type = TOKEN_INDENT;
						FirstToken = FALSE;
						return TOKEN_OK;
					}
				}

				break;

			case(STATE_DEDENT):

				dedentFound = FALSE;

				if ((*indent_stack)->value != currentIndent_static) // to samé, ale jen pro dedent a za pomoci statické proměnné, protože se generuje více tokenů naráz
				{
					if ((*indent_stack)->level == 1 && currentIndent_static != 0) // špatné odsazení
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					popStack(indent_stack);
					newToken->type = TOKEN_DEDENT;
					ungetc(currentChar, stdin);

					if ((*indent_stack)->value == currentIndent_static) // pokud najdu aktuální zanoření na stacku, zpracoval jsem aktuální dedenty
					{
						FirstToken = FALSE;
						dedentFound = TRUE;
					}

					if (dedentFound == FALSE && (*indent_stack)->value != currentIndent_static && (*indent_stack)->level == 1 && currentIndent_static != 0)
					{ // nenašel jsem aktuální zanoření, stack je prázdný - chyba
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					return TOKEN_OK;
				}

				if ((*indent_stack)->value == currentIndent_static)
				{
					dedentFound = TRUE;
					currentIndent_static = 0;
				}

				if (dedentFound == FALSE) // pokud při dedentech nenajdu aktuální zanoření na stacku - error
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
				else
				{
					newToken->type = TOKEN_DEDENT;
					FirstToken = FALSE;
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}

				break;

			case(STATE_DOC_STRING):

				if (currentChar == EOF)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}

				if (docStringCounter == 3) // pokud jsou 3x ", začátek stringu
				{
					if (currentChar == EOF)
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					state = STATE_DOC_STRING_END;
					ungetc(currentChar, stdin);
					docStringCounter = 0;
					break;
				}
				if (currentChar == '\"' && previousChar != '\\') // pokud předchozí znak není \, připočítat
				{
					if (docStringCounter < 3)
					{
						docStringCounter++;
					}
				}
				else if(currentChar != '\"' && docStringCounter < 3) // když je v začátku dok. řetězce jiný znak než ", error
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}

				break;

			case(STATE_DOC_STRING_END):

				if (docStringCounter == 3) // konec stringu
				{
					newToken->type = TOKEN_STRING;
					previousChar_static = 'y';
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}
				if (currentChar == EOF)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
				if (currentChar == '\"' && docStringCounter < 3 && previousChar_static != '\\') // " = pom +1
				{
					docStringCounter++;
				}
				else if (currentChar != '\"' && docStringCounter < 3) // nějaký jiný znak restartuje počítadlo
				{
					docStringCounter = 0;
					if (previousChar_static == '\"')
					{
						if(updateDynamicString('\"', newToken))
						{
							freeMemory(newToken, indent_stack);
							return INTERNAL_ERROR;
						}
					}
				}

				if(currentChar == '\"' && previousChar_static == '\\') // ošetření situace:  """ te " st \""""
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}

					previousChar_static = currentChar;
					break;
				}
				else if (currentChar != '\"')
				{
					if (currentChar == '\\')
					{
						previousChar_static = currentChar;
						break;
					}
					if (previousChar_static == '\\')
					{
						if(updateDynamicString('\\', newToken))
						{
							freeMemory(newToken, indent_stack);
							return INTERNAL_ERROR;
						}
					}
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
				}

				previousChar_static = currentChar;
				break;

			case(STATE_ERROR): // k tomuto by asi nemělo dojít, ale jistota je jistota

				freeMemory(newToken, indent_stack);
				return LEX_ERROR;
				break;

		}
		previousChar = currentChar;
	}

	freeMemory(newToken, indent_stack);
	return LEX_ERROR;
}

int preloadToken(TokenPTR* token, iStack* stack)
{
	int FirstToken_backup = FirstToken;
	int currentIndent_static_backup = currentIndent_static;
	char previousChar_static_backup = previousChar_static;

	fpos_t position;
	fgetpos(stdin, &position);
	int result = getToken(token , stack);
	fsetpos(stdin, &position);

	FirstToken = FirstToken_backup;
	currentIndent_static = currentIndent_static_backup;
	previousChar_static = previousChar_static_backup;
	
	return  result;
}

/** konec souboru "scanner.c" **/
