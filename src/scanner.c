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
 * 			 Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"

int debug = 1; // debug výpisy - 1 = zapnuto, 0 = vypnuto

void setSourceFile(FILE *f)
{
	source_f = f;
}

TokenPTR makeToken(TokenPTR* token) // vytvoří nový token a mallokuje základní prostor
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

	newToken->integer = 0;
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

void pushStack(iStack* indent_stack, int current_indent_value)
{
	iStack temp = (iStack) malloc(sizeof(struct indentStack));

	temp->value = current_indent_value;
	temp->level += (*indent_stack)->level + 1;
	temp->link = *indent_stack;

	*indent_stack = temp;
}

void popStack(iStack* indent_stack)
{
	iStack temp = (iStack) malloc(sizeof(struct indentStack));

	if (indent_stack == NULL || (*indent_stack)->value == 0)
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

int updateDynamicString(char currentChar, TokenPTR token) // TODO
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

void freeMemory(TokenPTR token)
{
	free(token->dynamic_value);
	free(token);
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

int getToken(TokenPTR* token, iStack* indent_stack) // + odkaz na stack?
{
	int state = STATE_START;
	int commentary_Counter;
	char currentChar, previousChar;

	int current_indent_value = 0;

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
					newToken->type = TOKEN_EOF;
					return TOKEN_OK;
				}
				else if (currentChar == '\\')
				{
					state = STATE_ESCAPE_SEQUENCE;
				}
				/*if (currentChar == '\n') // až bude hotové zpracování řetězců
				{
					newToken->type = TOKEN_EOL;
					return TOKEN_OK;
				}*/
				else if (currentChar == '\'' && previousChar != '\\')
				{
					state = STATE_STRING;
				}
				else if (currentChar == '#')
				{
					state = STATE_COMMENTARY;
				}
				else if (currentChar == '\"' && previousChar != '\\')
				{
					commentary_Counter = 1;
					state = STATE_BLOCK_COMMENTARY;
					if (debug)
					{
						printf("začátek blok. komentáře\n"); //debug
					}
				}
				else if (currentChar == '+')
				{
					newToken->type = TOKEN_PLUS;
					return TOKEN_OK;
				}
				else if (currentChar == '-')
				{
					newToken->type = TOKEN_MINUS;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == '*')
				{
					newToken->type = TOKEN_MUL;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == '<')
				{
					state = STATE_LESS_THAN;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '>')
				{
					state = STATE_MORE_THAN;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '!')
				{
					state = STATE_NOT_EQUAL;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '=')
				{
					state = STATE_ASSIGN;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '/')
				{
					state = STATE_DIV;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (currentChar == '(')
				{
					newToken->type = TOKEN_LEFT_BRACKET;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == ')')
				{
					newToken->type = TOKEN_RIGHT_BRACKET;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == ',')
				{
					newToken->type = TOKEN_COMMA;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (currentChar == ':')
				{
					newToken->type = TOKEN_COLON;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					return TOKEN_OK;
				}
				else if (isalpha(currentChar) || currentChar == '_')
				{
					state = STATE_IDENTIFIER;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (isdigit(currentChar))
				{
					state = STATE_NUMBER_INT;
					previousChar = currentChar;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (currentChar == ' ' && previousChar == '\n') // přeskočit mezery
				{
					state = STATE_INDENT;
				}
				else // přeskočí zbytečné mezery
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
 				}
				break;

			case(STATE_BLOCK_COMMENTARY):
				if (currentChar == EOF)
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
				
				previousChar = currentChar;
				
				if (commentary_Counter == 3)
				{
					if (currentChar == EOF)
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}

					state = STATE_BLOCK_COMMENTARY_END;
					commentary_Counter = 0;
					if (debug)
					{
						printf("text komentáře\n"); //debug
					}
					break;
				}
				if (currentChar == '\"' && previousChar != '\\')
				{
					if (commentary_Counter < 3)
					{
						commentary_Counter++;	
					}
				}
				else if(currentChar != '\"' && commentary_Counter < 3)
				{
					freeMemory(newToken);					
					return LEX_ERROR;
				}

				break;

			case(STATE_BLOCK_COMMENTARY_END):
				previousChar = currentChar;

				if (commentary_Counter == 3)
				{				
					state = STATE_START;
					commentary_Counter = 0;
					if (debug)
					{
						printf("konec blok. komentáře! \n"); //debug
					}
					break;
				}
				if (currentChar == EOF)
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
				if (currentChar == '\"' && commentary_Counter < 3 && previousChar != '\\')
				{
					commentary_Counter++;
				}
				else if (currentChar != '\"' && commentary_Counter < 3)
				{
					commentary_Counter = 0;
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
						freeMemory(newToken);
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
						freeMemory(newToken);
						return LEX_ERROR;
					}
					newToken->type = TOKEN_NOT_EQUAL;
					return TOKEN_OK;
				}
				else
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
			break;

			case(STATE_ASSIGN):
				
				if (currentChar == '=')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					newToken->type = TOKEN_EQUAL;
					return TOKEN_OK;
				}
				else
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
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
						freeMemory(newToken);
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
						freeMemory(newToken);
						return LEX_ERROR;
					}

					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					break;
				}
				else if (currentChar == '.')
				{
					state = STATE_NUMBER_DOUBLE;
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					break;
				}
				if (currentChar == 'e' || currentChar == 'E')
				{
					if(updateDynamicString(currentChar, newToken)) // TODO: pak dodělat dopočítání přesné hodnoty
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_NUMBER_EXPONENT;
					break;
				}
				else if (isalpha(currentChar))
				{
					freeMemory(newToken);
					return LEX_ERROR;
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
					if(updateDynamicString(currentChar, newToken)) // TODO: pak dodělat dopočítání přesné hodnoty
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_NUMBER_EXPONENT;
					break;
				}
				else if (isdigit(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					break;
				}
				else if (isalpha(currentChar))
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar != '\f')
				{
					newToken->type = TOKEN_DOUBLE;
					ungetc(currentChar, source_f);
					return TOKEN_OK;
				}

			break;
			case(STATE_NUMBER_EXPONENT): 
				if (currentChar == '+' || currentChar == '-')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					break;
				}
				else if (isdigit(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
				}
				else if (isalpha(currentChar))
				{
					freeMemory(newToken);
					return LEX_ERROR;
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
						freeMemory(newToken);
						return LEX_ERROR;
					}
					break;
				}
				else if (isalnum(currentChar))
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
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
						freeMemory(newToken);
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
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\'')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\n')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\t')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == '\\')
				{
					if(updateDynamicString(currentChar, newToken))
					{
						freeMemory(newToken);
						return LEX_ERROR;
					}
					state = STATE_START;
					break;
				}
				else if (currentChar == 'x') // hexa TODO
				{
					state = STATE_START;
					break;
				}
			break;

			case(STATE_INDENT): // TODO
			break;

			case(STATE_DEDENT): // TODO
			break;

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