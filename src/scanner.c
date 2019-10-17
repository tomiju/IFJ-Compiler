#include "scanner.h"

int debug = 1; // debug výpisy - 1 = zapnuto, 0 = vypnuto

void setSourceFile(FILE *f)
{
	source_f = f;
}

TokenPTR makeToken(TokenPTR* token)
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
	newToken->decimal = 0.0;

	return newToken;
}

void updateDynamicString(TokenPTR token) // TODO
{

}

void IntegerConcatenate(char a, TokenPTR token) // #YOLO funkce (probably fix? :D)
{ 
  
    char s1[2000000]; 
    char s2[2000000]; 
  
    // Convert both the integers to string 
    sprintf(s1, "%c", a);

    sprintf(s2, "%d", token->integer); 
  
    // Concatenate both strings 
    strcat(s2, s1); 
  
    // Convert the concatenated string 
    // to integer 
    token->integer = atoi(s2); 
}

int ExponentConcatenate(int number_Exponent , char b) 
{ 
  
    char s1[2000000]; 
    char s2[2000000]; 
  
    // Convert both the integers to string 
    sprintf(s1, "%c", b); 
    sprintf(s2, "%d", number_Exponent); 
  
    // Concatenate both strings 
    strcat(s2, s1); 
    // Convert the concatenated string 
    // to integer 
    
    int vysledek = atoi(s2);

    return vysledek;
 
}  

void freeMemory(TokenPTR token)
{
	free(token->dynamic_value);
	free(token);
}

int getToken(TokenPTR* token)
{
	int state = STATE_START;
	int commentary_Counter;
	int number_Exponent = 0;
	char currentChar;

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
				if (currentChar == EOF)
				{
					newToken->type = TOKEN_EOF;
					return TOKEN_OK;
				}
				/*if (currentChar == '\n') // až bude hotové zpracování řetězců
				{
					newToken->type = TOKEN_EOL;
					return TOKEN_OK;
				}*/
				else if (currentChar == '#')
				{
					state = STATE_COMMENTARY;
				}
				else if (currentChar == '\"')
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
					return TOKEN_OK;
				}
				else if (currentChar == '*')
				{
					newToken->type = TOKEN_MUL;
					return TOKEN_OK;
				}
				else if (currentChar == '<')
				{
					state = STATE_LESS_THAN;
				}
				else if (currentChar == '>')
				{
					state = STATE_MORE_THAN;
				}
				else if (currentChar == '!')
				{
					state = STATE_NOT_EQUAL;
				}
				else if (currentChar == '=')
				{
					state = STATE_ASSIGN;
				}
				else if (currentChar == '/')
				{
					state = STATE_DIV;
				}
				else if (isdigit(currentChar))
				{
					state = STATE_NUMBER_INT;
					IntegerConcatenate(currentChar, newToken);
				}
				else // debug - smazat!!!
				{
					if (debug)
					{
						printf("test znak: %c\n", currentChar); //debug
					}				
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
					//ungetc(currentChar, source_f); // kvůli EOF?
 				}
				break;

			case(STATE_BLOCK_COMMENTARY):
				if (currentChar == EOF)
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
				if (commentary_Counter == 3)
				{
					if (currentChar != '\n' && currentChar != ' ' && currentChar != '\v' && currentChar != '\t' && currentChar != EOF && currentChar != '\r' && currentChar != '\f')
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
				if (currentChar == '\"')
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
				if (commentary_Counter == 3)
				{				
					if (currentChar != '\n' && currentChar != ' ' && currentChar != '\v' && currentChar != '\t' && currentChar != EOF && currentChar != '\r' && currentChar != '\f')
					{
						freeMemory(newToken);
						return LEX_ERROR;	
					}
					
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
				if (currentChar == '\"' && commentary_Counter < 3)
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
					newToken->type = TOKEN_MORE_THAN_OR_EQUAL;
					return TOKEN_OK;
				}
			break;

			case(STATE_NOT_EQUAL):
				if (currentChar == '=')
				{
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
					// na uložení funkce, která zkontroluje alokovaný prostor
					IntegerConcatenate(currentChar, newToken);

				}
				else if (currentChar == '.')
				{
					state = STATE_NUMBER_FLOAT;
					break;
					/*ungetc(currentChar, source_f);
					newToken->type = TOKEN_INT;
					return TOKEN_OK;*/
				}
				else if (isalpha(currentChar))
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar != '\f')
				{
					newToken->type = TOKEN_INT;
					return TOKEN_OK;
				}
			break;
			
			case(STATE_NUMBER_FLOAT): // TODO!!!

				if (currentChar == 'e' || currentChar == 'E')
				{
					//todo
					/*if (currentChar == 'E')
					{
						currentChar = 'e';
					}*/
					state = STATE_NUMBER_EXPONENT;
					break;
				}

			break;
			case(STATE_NUMBER_EXPONENT): // pouze pro desetinné čísla => TODO!!!
				if (isdigit(currentChar))
				{
					
					number_Exponent = ExponentConcatenate(number_Exponent, currentChar);
					//printf("exponent1 %d\n",number_Exponent );
				}
				else if (currentChar == '+' || currentChar == '-')
				{
					// todo
				}
				else if (isalpha(currentChar))
				{
					freeMemory(newToken);
					return LEX_ERROR;
				}
				else if (currentChar == '\n' || currentChar == ' ' || currentChar == '\v' || currentChar == '\t' || currentChar == EOF || currentChar == '\r' || currentChar != '\f')
				{
					newToken->type = TOKEN_INT;
					printf("exponent %d\n",number_Exponent );
					IntegerConcatenate(number_Exponent, newToken); // takto asi ne, uložím rovnou výlednou hodnotu... - zvlášť funkce
					return TOKEN_OK;
				}
			break;

			break;
		}
	}

}