/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   semantic.c
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta   <xkucht09@stud.fit.vutbr.cz>
 */

#include <stdio.h>
#include "stdbool.h"
#include "scanner.h"
#include "semantic.h"
#include "parser.h"

#define TAB_SIZE 8


// Precedence table
int prec_table[TAB_SIZE][TAB_SIZE] =
{
//	(S < SHIFT) (E = EQUAL) (R > REDUCE) (N # ERROR)
//	|+- | */| \ | r | ( | ) | i | $ |
	{ R , S , S , R , S , R , S , R }, /// +-
	{ R , R , R , R , S , R , S , R }, /// */
	{ R , S , R , R , S , R , S , R }, /// \ /
	{ S , S , S , N , S , R , S , R }, /// r (realtion operators) = !> <= < >= >
	{ S , S , S , S , S , E , S , N }, /// (
	{ R , R , R , R , N , R , N , R }, /// )
	{ R , R , R , R , N , R , N , R }, /// i (id, int, double, string)
	{ S , S , S , S , S , N , S , N }  /// $
};

// konvertuje symbol na index do tabulky
Prec_table_index_enum get_prec_table_index(TokenTYPE symbol)
{
	switch (symbol)
	{
		case TOKEN_PLUS:
		case TOKEN_MINUS:
			return I_PLUS_MINUS;
	
		case TOKEN_MUL:
		case TOKEN_DIV:
			return I_MUL_DIV;
	
		case TOKEN_IDIV:
			return I_IDIV;
	
		case TOKEN_LESS_THAN:
		case TOKEN_MORE_THAN:
		case TOKEN_LESS_THAN_OR_EQUAL:
		case TOKEN_MORE_THAN_OR_EQUAL:
		case TOKEN_NOT_EQUAL:
		case TOKEN_EQUAL:
		case TOKEN_ASSIGN:
			return I_REL_OP;
	
		case TOKEN_LEFT_BRACKET:
			return I_LEFT_BRACKET;
	
		case TOKEN_RIGHT_BRACKET:
			return I_RIGHT_BRACKET;
	
		case TOKEN_INT:
		case TOKEN_DOUBLE:
		case TOKEN_STRING:
		case TOKEN_NONE:
		case TOKEN_IDENTIFIER:
		case TOKEN_KEYWORD:
		case KEYWORD_DEFAULT:
		case KEYWORD_IF:
		case KEYWORD_ELSE:
		case KEYWORD_RETURN:
		case KEYWORD_DEF:
		case KEYWORD_NONE:
		case KEYWORD_WHILE:
		case KEYWORD_INPUTS:
		case KEYWORD_INPUTI:
		case KEYWORD_INPUTF:
		case KEYWORD_PRINT:
		case KEYWORD_LEN:
		case KEYWORD_SUBSTR:
		case KEYWORD_ORD:
		case KEYWORD_CHR:
		case KEYWORD_PASS:
			return I_DATA;
	
		default:
			return I_DOLLAR;
	}
}

int expression(){
    printf("expression\n");
    int result;
    //sezere tri tokeny bez syntakticke kontroly
    
    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    return TOKEN_OK;
}











void initTokenStack(TStackToken *stack)
{
	stack->top = NULL;
}

void pushTokenStack(TStackToken* stack, TokenTYPE DataType, Prec_table_index_enum TokenType)
{
	TStackTokenItem new_item = (TStackTokenItem) malloc(sizeof(struct stacktokenitem)); 

	if (new_item == NULL)
	{
		return;
	}

	new_item->data_type = DataType;
	new_item->token_type = TokenType;
	new_item->next_token = stack->top;

	stack->top = new_item;

}

bool popTokenStack(TStackToken* stack)
{
	if (stack->top != NULL)
	{
		TStackTokenItem tmp = stack->top;
		stack->top = tmp->next_token;
		free(tmp);

		return TRUE;
	}
	return FALSE;
}

void destroyTokenStack(TStackToken* stack)
{
	while(popTokenStack(stack));
}
