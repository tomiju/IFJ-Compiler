/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   scanner.c
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"

int debug = 0; // debug výpisy - 1 = zapnuto, 0 = vypnuto

void setSourceFile(FILE *f)
{
	source_f = f;
}

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

/*int addStringToDynamicString(char* inputString, TokenPTR token)
{
	int inputStringLength = (int) strlen(inputString);

	if (token->size + inputStringLength + 1 >= token->allocated_size)
	{

		int realloc_size = token->size + inputStringLength + 1;
		token->dynamic_value = (char*) realloc(token->dynamic_value, realloc_size);

		if (token->dynamic_value == NULL)
		{
			return 1;
		}
		token->allocated_size = realloc_size;
	}

	token->size += inputStringLength;
	strcat(token->dynamic_value, inputString);
	token->dynamic_value[token->size] = '\0';

	return 0;
}*/

void computeNumberWithExponent(TokenPTR token)
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
	if ((pom = strcmp("inputs", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_INPUTS;
	 	return 0;
	}
	if ((pom = strcmp("inputi", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_INPUTI;
	 	return 0;
	}
	if ((pom = strcmp("inputf", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_INPUTF;
	 	return 0;
	}
	if ((pom = strcmp("print", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_PRINT;
	 	return 0;
	}
	if ((pom = strcmp("len", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_LEN;
	 	return 0;
	}
	if ((pom = strcmp("substr", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_SUBSTR;
	 	return 0;
	}
	if ((pom = strcmp("ord", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_ORD;
	 	return 0;
	}
	if ((pom = strcmp("chr", token->dynamic_value)) == 0)
	{
	 	token->type = KEYWORD_CHR;
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
	char currentChar, previousChar;

	static int FirstToken = TRUE;

	TokenPTR newToken = makeToken(token);
	
	if (newToken == NULL) // pokud byla chyba v alokaci -> error
	{
		return LEX_ERROR; // možná nějaký jiný???
	}

	while(42)
	{
		
		currentChar = (char) getc(source_f);

		switch(state)
		{
			case(STATE_START):
				
				previousChar = currentChar;

				if (currentChar == EOF)
				{
					if ((*indent_stack)->value != 0)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
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
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_ESCAPE_SEQUENCE;
					FirstToken = FALSE;
				}
				if (currentChar == '\n') // pro indent / dedent... nebo EOL - podle proměnné "FirstInLine"
				{
					if (FirstToken == FALSE)
					{
						newToken->type = TOKEN_EOL;
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
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_STRING;
					FirstToken = FALSE;
				}
				else if (currentChar == '#')
				{

					state = STATE_COMMENTARY;
					break;
				}
				else if (currentChar == '\"' && previousChar != '\\')
				{
					commentaryCounter = 1;
					state = STATE_BLOCK_COMMENTARY;
					if (debug)
					{
						printf("začátek blok. komentáře\n"); //debug
					}
				}
				else if (currentChar == '+')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_PLUS;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == '-')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_MINUS;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == '*')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_MUL;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == '<')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_LESS_THAN;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '>')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_MORE_THAN;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '!')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_NOT_EQUAL;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '=')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_ASSIGN;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '/')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_DIV;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '(')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_LEFT_BRACKET;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == ')')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_RIGHT_BRACKET;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == ',')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_COMMA;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == ':')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					newToken->type = TOKEN_COLON;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (isalpha(currentChar) || currentChar == '_')
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_IDENTIFIER;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (isdigit(currentChar))
				{
					if (currentIndent != (*indent_stack)->value && FirstToken == TRUE)
					{
						state = STATE_DEDENT;
						ungetc(currentChar, source_f);
						break;
					}
					state = STATE_NUMBER_INT;
					previousChar = currentChar;
					FirstToken = FALSE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (currentChar == ' ' && FirstToken == TRUE) // indent
				{
					state = STATE_INDENT;
					currentIndent++;
				}
				else // přeskočí zbytečné mezery (asi spíš ne, ale to je jedno)
				{
					break;		
				}

				break;

			case(STATE_COMMENTARY):
				if (currentChar == '\n' || currentChar == EOF)
				{
					state = STATE_START;

					if (debug)
					{
						printf("normální komentář\n"); //debug
					}

					ungetc(currentChar, source_f);
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
					commentaryCounter = 0;

					if (debug)
					{
						printf("text komentáře\n"); //debug
					}

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
					state = STATE_START;
					commentaryCounter = 0;
					
					if (debug)
					{
						printf("konec blok. komentáře! \n"); //debug
					}

					currentIndent = 0;
					break;
				}
				if (currentChar == EOF)
				{
					freeMemory(newToken, indent_stack);
					return LEX_ERROR;
				}
				if (currentChar == '\"' && commentaryCounter < 3 && previousChar != '\\')
				{
					commentaryCounter++;
				}
				else if (currentChar != '\"' && commentaryCounter < 3)
				{
					commentaryCounter = 0;
				}
				
			break;

			case(STATE_LESS_THAN):
				if (currentChar != '=')
				{
					ungetc(currentChar, source_f); // tento znak už není součástí tohoto tokenu, tudíž se vrátím zpět a zpracuji ho znova
					newToken->type = TOKEN_LESS_THAN;
					return TOKEN_OK;
				}
				else if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					newToken->type = TOKEN_LESS_THAN_OR_EQUAL;
					return TOKEN_OK;
				}
			break;

			case(STATE_MORE_THAN):
				if (currentChar != '=')
				{
					ungetc(currentChar, source_f); // tento znak už není součástí tohoto tokenu, tudíž se vrátím zpět a zpracuji ho znova
					newToken->type = TOKEN_MORE_THAN;
					return TOKEN_OK;
				}
				else if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
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
						return LEX_ERROR;
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
						return LEX_ERROR;
					}
					newToken->type = TOKEN_EQUAL;
					return TOKEN_OK;
				}
				else
				{
					ungetc(currentChar, source_f);
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
						return LEX_ERROR;
					}
					newToken->type = TOKEN_IDIV;
					return TOKEN_OK;
				}
				else
				{
					ungetc(currentChar, source_f);
					newToken->type = TOKEN_DIV;
					return TOKEN_OK;
				}
			break;

			case(STATE_NUMBER_INT):
				if (isdigit(currentChar))
				{
					if (currentChar != '0' && previousChar == '0') // ošetření přebytečných nul na začátku čísla
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					break;
				}
				else if (currentChar == '.')
				{
					state = STATE_NUMBER_DOUBLE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					break;
				}
				if (currentChar == 'e' || currentChar == 'E')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					state = STATE_NUMBER_EXPONENT;
					previousChar = currentChar;
					break;
				}
				else if (isalpha(currentChar))
				{
					newToken->type = TOKEN_INT;
					ungetc(currentChar, source_f);
					return TOKEN_OK;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar != '\f')
				{
					newToken->type = TOKEN_INT;
					ungetc(currentChar, source_f);
					return TOKEN_OK;
				}
			break;
			
			case(STATE_NUMBER_DOUBLE): // TODO!!!

				if (currentChar == 'e' || currentChar == 'E')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
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
						return LEX_ERROR;
					}
					break;
				}
				else if (isalpha(currentChar))
				{
					newToken->type = TOKEN_INT;
					ungetc(currentChar, source_f);
					return TOKEN_OK;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar != '\f')
				{
					newToken->type = TOKEN_DOUBLE;
					ungetc(currentChar, source_f);
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
						return LEX_ERROR;
					}
					break;
				}
				else if (isdigit(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
				}
				else if (isalpha(currentChar))
				{
					newToken->type = TOKEN_INT;
					ungetc(currentChar, source_f);
					return TOKEN_OK;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar != '\f')
				{
					newToken->type = TOKEN_DOUBLE;
					ungetc(currentChar, source_f);
					computeNumberWithExponent(newToken);
					return TOKEN_OK;
				}
			break;

			case(STATE_IDENTIFIER):
				if (currentChar == '_')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					break;
				}
				else if (isalnum(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					break;
				}
				else
				{
					if (!(checkKeyword(newToken)))
					{
						ungetc(currentChar, source_f);
						return TOKEN_OK;
					}
					
					ungetc(currentChar, source_f);
					newToken->type = TOKEN_IDENTIFIER;
					return TOKEN_OK;
				}
			break;

			case(STATE_STRING):


				if (currentChar == '\'' && previousChar != '\\')
				{
					newToken->type = TOKEN_STRING;
					return TOKEN_OK;
				}
				else if (currentChar == EOF)
				{
					return LEX_ERROR;
				}
				else if (currentChar == '\\')
				{
					previousChar = currentChar;
					break;
				}
				else
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					previousChar = currentChar;
					break;
				}
				previousChar = currentChar;
			break;

			case(STATE_ESCAPE_SEQUENCE): // TODO?
				if (currentChar == '\"')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\'')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\n')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\t')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\\')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken, indent_stack);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == 'x') // hexa TODO??
				{
					state = STATE_START;
					break;
				}
			break;

			case(STATE_INDENT):
				
				if (currentChar == ' ')
				{
					currentIndent++;
					break;
				}
				
				if ((currentChar == '\n' || currentChar == '#' || currentChar == '\"') && FirstToken == TRUE)
				{
					currentIndent = 0;
					ungetc(currentChar, source_f);
					state = STATE_START;
					break;
				}
				else
				{
					if ((*indent_stack)->value == currentIndent)
					{
						newToken->type = TOKEN_NO_INDENT_OR_DEDENT;
						FirstToken = FALSE;
						ungetc(currentChar, source_f);	
						return TOKEN_OK;
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
							ungetc(currentChar, source_f);

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
						}

						if (dedentFound == FALSE)
						{
							freeMemory(newToken, indent_stack);
							return LEX_ERROR;
						}
						else
						{
							newToken->type = TOKEN_DEDENT;	
							ungetc(currentChar, source_f);
							FirstToken = FALSE;
							return TOKEN_OK;
						}
					}
					else
					{
						pushStack(indent_stack, currentIndent);
						ungetc(currentChar, source_f);
						newToken->type = TOKEN_INDENT;
						
						FirstToken = FALSE;
						
						return TOKEN_OK;
					}
				}
				
			break;

			case(STATE_DEDENT):

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
					ungetc(currentChar, source_f);

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
					ungetc(currentChar, source_f);
					return TOKEN_OK;
				}

			break;
		}
		previousChar = currentChar;
	}

	if (debug)
	{
		printf("konec scanneru error\n");
	}

	return LEX_ERROR;
}

int preloadToken(TokenPTR* token, iStack* stack)
{
	fpos_t position;

	fgetpos(source_f, &position);

	int result = getToken( token , stack);

	fsetpos(source_f, &position);

	return  result;
}

/** konec souboru "scanner.c" **/