#include "scanner.h"

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
	return newToken;
}

void updateDynamicString(TokenPTR token)
{

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
	char currentChar;

	TokenPTR newToken = makeToken(token);
	
	if (newToken == NULL)
	{
		return LEX_ERROR;
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
				else if (currentChar == '#')
				{
					state = STATE_COMMENTARY;
				}
				else if (currentChar == '\"')
				{
					commentary_Counter = 1;
					state = STATE_BLOCK_COMMENTARY;
					printf("začátek blok. komentáře\n");
				}
				else
				{
					printf("test znak: %c\n", currentChar);
				}
				break;

			case(STATE_COMMENTARY):
				if (currentChar == '\n' || currentChar == EOF)
				{
					state = STATE_START;
					printf("normální komentář\n");
					ungetc(currentChar, source_f); // kvůli EOF?
 				}
				break;

			case(STATE_BLOCK_COMMENTARY):
				if (currentChar == EOF)
				{
					freeMemory(newToken);
					printf("err1\n");
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
						printf("text komentáře\n");
						break;
				}
				if (currentChar == '\"')
				{
					if (commentary_Counter < 3)
					{
						commentary_Counter++;	
					}
				}
				else if(currentChar != '\"')
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
						printf("konec blok. komentáře! \n");
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


			break;
		}
	}

}