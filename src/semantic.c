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
#define SYNTAX_OK 0


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

extern int currentLine;

int expression(TokenTYPE *expression_type){
    printf("expression\n");
    int result;
    // sezere tri tokeny bez syntakticke kontroly
    
    // result = getToken(&token_ptr, &indent_stack );
    // if(result != TOKEN_OK)return result;

    // result = getToken(&token_ptr, &indent_stack );
    // if(result != TOKEN_OK)return result;

    // result = getToken(&token_ptr, &indent_stack );
    // if(result != TOKEN_OK)return result;

    TStackToken *Stack = (TStackToken*) malloc(sizeof(struct stacktoken));
    if (Stack == NULL)
    {
    	printf("Chyba alokácie\n");
    	return 1;
    }

    initTokenStack(Stack);
    pushTokenStack(Stack, TOKEN_DOLLAR, I_DOLLAR);


    bool success = FALSE;
    int count;
    TStackTokenItem tmpitem;
    

    do
    {	
    	
    	switch(prec_table[Stack->top->token_type][get_prec_table_index(token_ptr->type)])
    	{
		case S:
			// printf("OPERATION S\n");

			if (get_prec_table_index(token_ptr->type) == I_DATA)
			{
				pushTokenStack(Stack, TOKEN_STOP, I_STOP);
			}
			pushTokenStack(Stack, token_ptr->type, get_prec_table_index(token_ptr->type));

			if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
    		    fprintf(stderr,"line: %d\n",currentLine);
    		    return LEX_ERROR;
    		}

			// TODO generate code

			break;

		case E:
			// printf("OPERATION E\n");
			pushTokenStack(Stack, TOKEN_RIGHT_BRACKET, I_RIGHT_BRACKET);

			if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
    		    fprintf(stderr,"line: %d\n",currentLine);
    		    return LEX_ERROR;
    		}

			break;

		case R:
			// printf("OPERATION R\n");
			result = reduce(Stack);

			if(result != SYNTAX_OK)
			{
				// TODO FREE
				return result;
			}

			break;

		case N:
			// printf("OPERATION N\n");
			if (Stack->top->next_token->data_type == TOKEN_DOLLAR && get_prec_table_index(token_ptr->type) == I_DOLLAR)
			{
				success = TRUE;
			}

			// TODO FREE
			return SYNTAX_ERROR;

			break;
		}

		count = totalCountTokenStack(Stack);
    	if (count == 1 && get_prec_table_index(token_ptr->type) == I_DOLLAR)
    	{
    		success = TRUE;
    	}


    	tmpitem = Stack->top;
    	// printf("TOKENY NA STACKU: ");
    	while(tmpitem->token_type != I_DOLLAR)
    	{
    		// printf("%d|%d ", tmpitem->data_type, tmpitem->token_type);
    		tmpitem = tmpitem->next_token;
    	}
    	// printf("  Ďalší token: %s\n", token_ptr->dynamic_value);

    } while(success == FALSE);


    // printf("\n\n");

    tmpitem = Stack->top;
    	// printf("TOKENY NA STACKU: ");
    	while(tmpitem->token_type != I_DOLLAR)
    	{
    		// printf("%d|%d ", tmpitem->data_type, tmpitem->token_type);
    		tmpitem = tmpitem->next_token;
    	}

    // printf("\n\n");

 //    printf("END OF EXPRESSION, final data type: %d\n", Stack->top->data_type);

	// printf("Ďalší token: %s\n", token_ptr->dynamic_value);

	// printf("TOKEN_OK %d \n", TOKEN_OK);

	switch(Stack->top->data_type)
	{
		case TOKEN_NONTERM_INT:
			*expression_type = TOKEN_INT;
			break;
		case TOKEN_NONTERM_DOUBLE:
			*expression_type = TOKEN_DOUBLE;
			break;
		case TOKEN_NONTERM_STRING:
			*expression_type = TOKEN_STRING;
			break;
		case TOKEN_NONTERM_BOOL:
			*expression_type = TOKEN_NONTERM_BOOL;
			break;
		default:
			*expression_type = TOKEN_NONE;
	}

	return TOKEN_OK;
    // return Stack->top->data_type;
}

int reduce(TStackToken *stack)
{
	int count = countTokenStack(stack);
	int result;
	Prec_rules_enum gen_rule;
	TokenTYPE final_token_type;
	TStackTokenItem op1 = NULL;
	TStackTokenItem op2 = NULL;
	TStackTokenItem op3 = NULL;
	TStackTokenItem tmpitem = NULL;
	if (count == 1)
	{
		op1 = stack->top;
		if (op1->data_type >= TOKEN_NONTERM && op1->data_type <= TOKEN_NONTERM_BOOL)
		{
			tmpitem = op1->next_token;
			op1->next_token = tmpitem->next_token;
			free(tmpitem);

			count = countTokenStack(stack);

			if (count == 3)
			{
				result = reduce(stack);
				if (result != SYNTAX_OK)
				{
					return result;
				}
			}

			if (get_prec_table_index(token_ptr->type) != I_DOLLAR)
			{
				pushTokenStack(stack, token_ptr->type, get_prec_table_index(token_ptr->type));

				if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
				    fprintf(stderr,"line: %d\n",currentLine);
				    return LEX_ERROR;
				}
			}

			return SYNTAX_OK;
		}
		else
		{
			gen_rule = test_rule(count, op1, NULL, NULL);
		}
		
	}

	else if (count == 3)
	{
		op1 = stack->top->next_token->next_token;
		op2 = stack->top->next_token;
		op3 = stack->top;

		if (prec_table[op2->token_type][get_prec_table_index(token_ptr->type)] == S)
		{	

			TStackTokenItem new_item = (TStackTokenItem) malloc(sizeof(struct stacktokenitem)); 

			if (new_item == NULL)
			{
				printf("Alokacia sa nepodarila\n");
				return -1;
			}
		
			new_item->data_type = TOKEN_STOP;
			new_item->token_type = I_STOP;
			new_item->next_token = op2;
			op3->next_token = new_item;

			return SYNTAX_OK;
		}

		gen_rule = test_rule(count, op1, op2, op3);

	}

	else
	{
		return SYNTAX_ERROR;
	}

	if (gen_rule == NOT_A_RULE)
	{
		return SYNTAX_ERROR;
	}
	else
	{
		//TODO 
		// kontrola semantiky
		// generovanie kodu

		result = semantic(op1, op2, op3, gen_rule, &final_token_type);
		if (result == SEMANTIC_TYPE_COMPATIBILITY_ERROR)
		{
			return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
		}

		for (int i = 0; i < count; i++)
		{
			popTokenStack(stack);
		}

		pushTokenStack(stack, final_token_type, I_DATA);
	}

	return SYNTAX_OK;
}


int semantic(TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3, Prec_rules_enum rule, TokenTYPE *final_token_type)
{
	bool op1_to_double = false;
	bool op3_to_double = false;
	bool op1_to_integer = false;
	bool op3_to_integer = false;


	switch (rule)
	{
		case OPERAND:
			switch (op1->data_type)
			{
				case TOKEN_INT:
					*final_token_type = TOKEN_NONTERM_INT;
					break;

				case TOKEN_DOUBLE:
					*final_token_type = TOKEN_NONTERM_DOUBLE;
					break;

				case TOKEN_STRING:
					*final_token_type = TOKEN_NONTERM_STRING;
					break;

				case TOKEN_IDENTIFIER:
					*final_token_type = TOKEN_NONTERM_IDENTIFIER;
					break;

				default:
					break;
			}

			break;
	
		case LBR_NT_RBR:
			*final_token_type = op2->data_type;
			break;
	
		case NT_PLUS_NT:
		case NT_MINUS_NT:
		case NT_MUL_NT:
			if (op1->data_type == TOKEN_NONTERM_STRING && op3->data_type == TOKEN_NONTERM_STRING && rule == NT_PLUS_NT)
			{
				*final_token_type = TOKEN_NONTERM_STRING;
				break;
			}
	
			if (op1->data_type == TOKEN_NONTERM_INT && op3->data_type == TOKEN_NONTERM_INT)
			{
				*final_token_type = TOKEN_NONTERM_INT;
				break;
			}
	
			if (op1->data_type == TOKEN_NONTERM_STRING || op3->data_type == TOKEN_NONTERM_STRING)
				return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			*final_token_type = TOKEN_NONTERM_DOUBLE;
	
			if (op1->data_type == TOKEN_NONTERM_INT)
				op1_to_double = true;
	
			if (op3->data_type == TOKEN_NONTERM_INT)
				op3_to_double = true;
	
			break;
	
		case NT_DIV_NT:
			*final_token_type = TOKEN_NONTERM_DOUBLE;
	
			if (op1->data_type == TOKEN_NONTERM_STRING || op3->data_type == TOKEN_NONTERM_STRING)
				return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			if (op1->data_type == TOKEN_NONTERM_INT)
				op1_to_double = true;
	
			if (op3->data_type == TOKEN_NONTERM_INT)
				op3_to_double = true;
	
			break;
	
		case NT_IDIV_NT:
			*final_token_type = TOKEN_NONTERM_INT;
	
			if (op1->data_type == TOKEN_NONTERM_STRING || op3->data_type == TOKEN_NONTERM_STRING)
				return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			if (op1->data_type == TOKEN_NONTERM_DOUBLE)
				op1_to_integer = true;
	
			if (op3->data_type == TOKEN_NONTERM_DOUBLE)
				op3_to_integer = true;
	
			break;
	
		case NT_EQ_NT:
			// printf("SOM TU\n");
			*final_token_type = op1->data_type;
			break;

		case NT_NEQ_NT:
		case NT_LEQ_NT:
		case NT_LTN_NT:
		case NT_MEQ_NT:
		case NT_MTN_NT:
			*final_token_type = TOKEN_NONTERM_BOOL;
	
			if (op1->data_type == TOKEN_NONTERM_INT && op3->data_type == TOKEN_NONTERM_DOUBLE)
				op1_to_double = true;
	
			else if (op1->data_type == TOKEN_NONTERM_DOUBLE && op3->data_type == TOKEN_NONTERM_INT)
				op3_to_double = true;
	
			else if (op1->data_type != op3->data_type)
				return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			break;
	
		default:
			break;


	return 0;
	}
/*
	if (op1_to_double)
	{
		GENERATE_CODE(generate_stack_op2_to_double);
	}

	if (op3_to_double)
	{
		GENERATE_CODE(generate_stack_op1_to_double);
	}

	if (op1_to_integer)
	{
		GENERATE_CODE(generate_stack_op2_to_integer);
	}

	if (op3_to_integer)
	{
		GENERATE_CODE(generate_stack_op1_to_integer);
	}
*/
	return SYNTAX_OK;
}



Prec_rules_enum test_rule(int count, TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3)
{
	switch (count)
	{
	case 1:
		// rule E -> i
		if (op1->data_type == TOKEN_INT || op1->data_type == TOKEN_DOUBLE || op1->data_type == TOKEN_STRING || op1->data_type == TOKEN_IDENTIFIER) 
		{
			return OPERAND;
		}

		return NOT_A_RULE;

	case 3:
		// rule E -> (E)
		if (op1->data_type == TOKEN_LEFT_BRACKET && (op2->data_type == TOKEN_NONTERM_INT || op2->data_type == TOKEN_NONTERM_DOUBLE ||op2->data_type == TOKEN_NONTERM_STRING || TOKEN_NONTERM_IDENTIFIER) && op3->data_type == TOKEN_LEFT_BRACKET)
			return LBR_NT_RBR;

		if ((op1->data_type == TOKEN_NONTERM_INT || op1->data_type == TOKEN_NONTERM_DOUBLE ||op1->data_type == TOKEN_NONTERM_STRING || TOKEN_NONTERM_IDENTIFIER) && (op3->data_type == TOKEN_NONTERM_INT || op3->data_type == TOKEN_NONTERM_DOUBLE ||op3->data_type == TOKEN_NONTERM_STRING || TOKEN_NONTERM_IDENTIFIER))
		{
			switch (op2->data_type)
			{
			// rule E -> E + E
			case TOKEN_PLUS:
				return NT_PLUS_NT;

			// rule E -> E - E
			case TOKEN_MINUS:
				return NT_MINUS_NT;

			// rule E -> E * E
			case TOKEN_MUL:
				return NT_MUL_NT;

			// rule E -> E / E
			case TOKEN_DIV:
				return NT_DIV_NT;

			// rule E -> E \ E
			case TOKEN_IDIV:
				return NT_IDIV_NT;

			// rule E -> E = E
			case TOKEN_ASSIGN:
				return NT_EQ_NT;

			// rule E -> E <> E
			case TOKEN_NOT_EQUAL:
				return NT_NEQ_NT;

			// rule E -> E <= E
			case TOKEN_LESS_THAN_OR_EQUAL:
				return NT_LEQ_NT;

			// rule E -> E < E
			case TOKEN_LESS_THAN:
				return NT_LTN_NT;

			// rule E -> E >= E
			case TOKEN_MORE_THAN_OR_EQUAL:
				return NT_MEQ_NT;

			// rule E -> E > E
			case TOKEN_MORE_THAN:
				return NT_MTN_NT;

			// invalid operator
			default:
				return NOT_A_RULE;
			}
		}
		return NOT_A_RULE;
	}
	return NOT_A_RULE;
}




int countTokenStack(TStackToken *stack)
{
	int count = 0;
	TStackTokenItem item = stack->top;

	while(item->token_type != I_DOLLAR && item->token_type != I_STOP)
	{
		count++;
		item = item->next_token;
	}

	return count;
}


int totalCountTokenStack(TStackToken *stack)
{
	int count = 0;
	TStackTokenItem item = stack->top;

	while(item->token_type != I_DOLLAR)
	{
		count++;
		item = item->next_token;
	}

	return count;
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
		printf("Alokacia sa nepodarila\n");
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
