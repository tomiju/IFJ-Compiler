/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   scanner.c
 *
 *
 * Datum:    30.11.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"

int debug = 0; // debug výpisy - 1 = zapnuto, 0 = vypnuto

TokenPTR makeToken(TokenPTR* token) // vytvoří nový token a alokuje základní prostor
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

int getToken(TokenPTR* token, iStack* indent_stack) // + odkaz na stack?
{
	int state = STATE_START;
	int commentaryCounter;
	int dedentFound; // TRUE = na stacku byla hodnota zanoření (při DEDENT), FALSE = na stacku nebyla -> error
	int currentIndent = 0;
	static int currentIndent_static;
	char currentChar, previousChar, firstNumber;
	static char StaticPrevChar; // pro \n v řetězci
	static int FirstToken = TRUE;

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
					if ((*indent_stack)->value != 0)
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
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
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
					if (FirstToken == FALSE)
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
						FirstToken = FALSE;
						state = STATE_DOC_STRING;
						commentaryCounter = 1;
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
					break;
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
					break;
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
					break;
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
					break;
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

			case(STATE_BLOCK_COMMENTARY):
				if (currentChar == EOF)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}

				previousChar = currentChar;

				if (commentaryCounter == 3)
				{
					if (currentChar == EOF)
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					state = STATE_BLOCK_COMMENTARY_END;
					ungetc(currentChar, stdin);
					commentaryCounter = 0;
					break;
				}
				if (currentChar == '\"' && previousChar != '\\')
				{
					if (commentaryCounter < 3)
					{
						commentaryCounter++;
					}
				}
				else if(currentChar != '\"' && commentaryCounter < 3)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}

				break;

			case(STATE_BLOCK_COMMENTARY_END):

				previousChar = currentChar;

				if (commentaryCounter == 3)
				{
					if (currentChar == '\n' || currentChar == EOF)
					{
						state = STATE_START;
						commentaryCounter = 0;
						currentIndent = 0;
						break;
					}

					if (currentChar != ' ' && currentChar != '\v' && currentChar != '\t' && currentChar != '\r' && currentChar != '\f')
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					break;
				}
				if (currentChar == EOF)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
				if (currentChar == '\"' &&  previousChar != '\\')
				{
					if (commentaryCounter < 3)
					{
						commentaryCounter++;
					}
				}
				else if (currentChar != '\"' && commentaryCounter < 3)
				{
					commentaryCounter = 0;
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
				else if (!isdigit(currentChar))
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

			case(STATE_NUMBER_DOUBLE): // TODO!!!

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
				if ((currentChar == '+' || currentChar == '-') && (previousChar == 'e' || previousChar == 'E'))
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
					if (!(checkKeyword(newToken)))
					{
						ungetc(currentChar, stdin);
						return TOKEN_OK;
					}

					ungetc(currentChar, stdin);
					newToken->type = TOKEN_IDENTIFIER;
					return TOKEN_OK;
				}
			break;

			case(STATE_STRING):

				if (currentChar == 'n' && StaticPrevChar == '\\')
				{
					if(updateDynamicString('\n', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					StaticPrevChar = currentChar;
					break;
				}

				if (currentChar == '\"' && StaticPrevChar == '\\')
				{
					if(updateDynamicString('\"', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					StaticPrevChar = currentChar;
					break;
				}

				if (currentChar == '\'' && StaticPrevChar == '\\')
				{
					if(updateDynamicString('\'', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					StaticPrevChar = currentChar;
					break;
				}

				if (currentChar == 't' && StaticPrevChar == '\\')
				{
					if(updateDynamicString('\t', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					StaticPrevChar = currentChar;
					break;
				}

				if (currentChar == '\\' && StaticPrevChar == '\\')
				{
					if(updateDynamicString('\\', newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					StaticPrevChar = 'y';
					break;
				}

				if (currentChar == 'x' && StaticPrevChar == '\\')
				{
					currentChar = (char) getc(stdin);
					if (isdigit(currentChar) || (currentChar >= (char)65 && currentChar <= (char)70) || (currentChar >= (char)97 && currentChar <= (char)102))
					{
						StaticPrevChar = currentChar;
						state = STATE_ESCAPE_SEQUENCE;
						break;
					}
					else
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}

				if (currentChar == '\'' && StaticPrevChar != '\\') // otestovat
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
					StaticPrevChar = currentChar;
					break;
				}
				else
				{
					if (StaticPrevChar == '\\')
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
 					 StaticPrevChar = currentChar;
					 break;
					}
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					previousChar = currentChar;
					StaticPrevChar = currentChar;
					break;
				}
				previousChar = currentChar;
				StaticPrevChar = currentChar;
			break;

			case(STATE_ESCAPE_SEQUENCE): // TODO

				if (isdigit(currentChar) || (currentChar >= (char)65 && currentChar <= (char)70) || (currentChar >= (char)97 && currentChar <= (char)102))
				{
						char tmp[2];
						tmp[0] = StaticPrevChar;
						tmp[1] = currentChar;

						int tmpToChar = (int)strtol(tmp, NULL, 16);

						if(updateDynamicString((char)tmpToChar, newToken))
						{
							freeMemory(newToken, indent_stack);
							return INTERNAL_ERROR;
						}

						currentChar = getc(stdin);

						if(currentChar != '\'' && currentChar != ' ')
						{
							freeMemory(newToken, indent_stack);
							return LEX_ERROR;
						}
						ungetc(currentChar, stdin);
						state = STATE_STRING;
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

				if ((currentChar == '\n' || currentChar == '#' || currentChar == '\"' || currentChar == EOF) && FirstToken == TRUE)
				{
					currentIndent = 0;
					ungetc(currentChar, stdin);
					state = STATE_START;
					break;
				}
				else
				{
					if ((*indent_stack)->value == currentIndent)
					{
						state = STATE_START;
						ungetc(currentChar, stdin);
						break;
					}
					else if ((*indent_stack)->value > currentIndent)
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
							if ((*indent_stack)->value == currentIndent)
							{
								FirstToken = FALSE;
								dedentFound = TRUE;
							}

							if (dedentFound == FALSE && (*indent_stack)->value != currentIndent && (*indent_stack)->level == 1 && currentIndent != 0)
							{
								freeMemory(newToken, indent_stack);
								return LEX_ERROR;
							}

							return TOKEN_OK;
						}


						if ((*indent_stack)->value == currentIndent)
						{
							dedentFound = TRUE;
							currentIndent_static = 0;
						}

						if (dedentFound == FALSE)
						{
							freeMemory(newToken, indent_stack);
							return LEX_ERROR;
						}
						else
						{
							newToken->type = TOKEN_DEDENT;
							ungetc(currentChar, stdin);
							FirstToken = FALSE;
							return TOKEN_OK;
						}
					}
					else
					{
						pushStack(indent_stack, currentIndent);
						ungetc(currentChar, stdin);
						newToken->type = TOKEN_INDENT;

						FirstToken = FALSE;

						return TOKEN_OK;
					}
				}

			break;

			case(STATE_DEDENT):

				dedentFound = FALSE;
				if ((*indent_stack)->value != currentIndent_static)
				{
					if ((*indent_stack)->level == 1 && currentIndent_static != 0)
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					popStack(indent_stack);
					newToken->type = TOKEN_DEDENT;
					ungetc(currentChar, stdin);

					if ((*indent_stack)->value == currentIndent_static)
					{
						FirstToken = FALSE;
						dedentFound = TRUE;
					}

					if (dedentFound == FALSE && (*indent_stack)->value != currentIndent_static && (*indent_stack)->level == 1 && currentIndent_static != 0)
					{
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

				if (dedentFound == FALSE)
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

				if (commentaryCounter == 3)
				{
					if (currentChar == EOF)
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					state = STATE_DOC_STRING_END;
					ungetc(currentChar, stdin);
					commentaryCounter = 0;
					break;
				}
				if (currentChar == '\"' && previousChar != '\\')
				{
					if (commentaryCounter < 3)
					{
						commentaryCounter++;
					}
				}
				else if(currentChar != '\"' && commentaryCounter < 3)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
			break;

			case(STATE_DOC_STRING_END):

				if (commentaryCounter == 3)
				{
					newToken->type = TOKEN_STRING;
					ungetc(currentChar, stdin);
					return TOKEN_OK;
				}
				if (currentChar == EOF)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
				if (currentChar == '\"' && commentaryCounter < 3 && StaticPrevChar != '\\')
				{
					commentaryCounter++;
				}
				else if (currentChar != '\"' && commentaryCounter < 3)
				{
					commentaryCounter = 0;
					if (StaticPrevChar == '\"')
					{
						if(updateDynamicString('\"', newToken))
						{
							freeMemory(newToken, indent_stack);
							return INTERNAL_ERROR;
						}
					}
				}

				if(currentChar == '\"' && StaticPrevChar == '\\')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return INTERNAL_ERROR;
					}
					StaticPrevChar = currentChar;
					break;
				}
				else if (currentChar != '\"')
				{
					if (currentChar == '\\')
					{
						StaticPrevChar = currentChar;
						break;
					}
					if (StaticPrevChar == '\\')
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

			StaticPrevChar = currentChar;
			break;

			case(STATE_ERROR):
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
	fpos_t position;

	fgetpos(stdin, &position);

	int result = getToken( token , stack);

	fsetpos(stdin, &position);

	return  result;
}

/** konec souboru "scanner.c" **/
