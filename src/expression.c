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
#include <string.h>
#include "stdbool.h"
#include "scanner.h"
#include "expression.h"
#include "parser.h"
#include "symtable.h"
#include "generator.h"

#define TAB_SIZE 8
#define SYNTAX_OK 0

int name_counter = 0;


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

int expression(htab_item_t* htab_symbol)
{
    printf("expression\n");
    int result;
    bool success = FALSE;
    int count;
    // TStackTokenItem tmpitem;


    TStackToken *Stack = (TStackToken*) malloc(sizeof(struct stacktoken));
    if (Stack == NULL)
    {
    	return INTERNAL_ERROR;
    }

    initTokenStack(Stack);
    result = pushTokenStack(Stack, TOKEN_DOLLAR, I_DOLLAR, NULL);
    if (result != SYNTAX_OK)
    {
    	return result;
    }


    do
    {	
    	
    	switch(prec_table[Stack->top->token_type][get_prec_table_index(token_ptr->type)])
    	{
		case S:
			// printf("OPERATION S\n");

			result = shift(Stack);

			if(result != SYNTAX_OK)
			{
				// TODO FREE
				return result;
			}


			// TODO generate code

			break;

		case E:
			// printf("OPERATION E\n");
			result = pushTokenStack(Stack, TOKEN_RIGHT_BRACKET, I_RIGHT_BRACKET, NULL);
			if (result != SYNTAX_OK)
    		{
    			return result;
    		}

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


    	// tmpitem = Stack->top;
    	// printf("TOKENY NA STACKU: ");
    	// while(tmpitem->token_type != I_DOLLAR)
    	// {
    	// 	printf("%d|%d ", tmpitem->data_type, tmpitem->token_type);
    	// 	tmpitem = tmpitem->next_token;
    	// }
    	// printf("  Ďalší token: %s\n", token_ptr->dynamic_value);

    } while(success == FALSE);



	htab_symbol = Stack->top->table_symbol;

    // printf("END OF EXPRESSION, final data type: %d\n", *expression_type);
    // printf("\n\n");
    destroyTokenStack(Stack);

	return TOKEN_OK;
    // return Stack->top->data_type;
}

int shift(TStackToken *stack)
{
	int datovy_typ;
    int int_value;
    int result;
    double double_value;
    bool found = FALSE;
    htab_item_t* htab_symbol = NULL;


	if (get_prec_table_index(token_ptr->type) == I_DATA || get_prec_table_index(token_ptr->type) == I_LEFT_BRACKET)
	{
		// printf("PUSHUJEM: TOKEN_STOP\n");
		result = pushTokenStack(stack, TOKEN_STOP, I_STOP, NULL);
		if (result != SYNTAX_OK)
    	{
    		return result;
    	}
	}

	if (token_ptr->type == TOKEN_IDENTIFIER)
	{
		if (localSymtable != NULL)
		{
			htab_symbol = htab_find(localSymtable, token_ptr->dynamic_value);
			if (htab_symbol != NULL)
			{
				found = TRUE;
			}
		}

		if (found == FALSE)
		{
			htab_symbol = htab_find(globalSymtable, token_ptr->dynamic_value);
			if (htab_symbol != NULL)
			{
				found = TRUE;
			}
		}

		if (found == FALSE)
		{
			return SEMANTIC_UNDEF_VALUE_ERROR;
		}

		switch(htab_symbol->type)
		{
			case INT:
				token_ptr->type = TOKEN_INT;
				break;
			case FLOAT:
				token_ptr->type = TOKEN_DOUBLE;
				break;
			case STRING:
				token_ptr->type = TOKEN_STRING;
				break;
			// case UNKNOWN:
			// 	token_ptr->type = 
			// 	break;
			default:
				return SYNTAX_ERROR;
				break;
		}
	}

	else if (token_ptr->type >= TOKEN_INT && token_ptr->type <= TOKEN_STRING)
	{
		char* constant_name = get_name(); 
		if (constant_name == NULL)
		{
			return INTERNAL_ERROR;
		}

		switch(token_ptr->type)
		{
			case TOKEN_INT:
				datovy_typ = INT;
				break;
			case TOKEN_DOUBLE:
				datovy_typ = FLOAT;
				break;
			case TOKEN_STRING:
				datovy_typ = STRING;
				break;
			default:
				break;
		}

		htab_symbol = make_const(constant_name, datovy_typ);
		
		switch(token_ptr->type)
		{
			case TOKEN_INT:
				sscanf(token_ptr->dynamic_value, "%d", &int_value);
				htab_symbol->ival = int_value;
				break;
			case TOKEN_DOUBLE:
				sscanf(token_ptr->dynamic_value, "%lf", &double_value);
				htab_symbol->dval = double_value;
				break;
			case TOKEN_STRING:
				htab_symbol->sval = token_ptr->dynamic_value;
				break;
			default:
				break;
		}
	}


	// printf("PUSHUJEM: %s\n", token_ptr->dynamic_value);
	result = pushTokenStack(stack, token_ptr->type, get_prec_table_index(token_ptr->type), htab_symbol);
	if (result != SYNTAX_OK)
    {
    	return result;
    }


	if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }

    return SYNTAX_OK;
}

int reduce(TStackToken *stack)
{
	int count = countTokenStack(stack);
	int result;
	bool change = FALSE;
	Prec_rules_enum gen_rule;
	TokenTYPE final_token_type;
	TStackTokenItem op1 = NULL;
	TStackTokenItem op2 = NULL;
	TStackTokenItem op3 = NULL;
	TStackTokenItem op4 = NULL;
	TStackTokenItem tmpitem = NULL;
    htab_item_t* htab_symbol = NULL;

	

	if (stack->top->token_type == I_RIGHT_BRACKET)
	{
		if (stack->top->next_token->token_type == I_DOLLAR || stack->top->next_token->next_token->token_type == I_DOLLAR)
		{
			// printf("BRACKET SYNTAX_ERROR\n");
			return SYNTAX_ERROR;
		}

		op1 = stack->top->next_token->next_token;	// (
		op2 = stack->top->next_token;	// NT
		op3 = stack->top;	// )

		op2->next_token = op1->next_token;
		stack->top = op2;
		free(op1);
		free(op3);

		op1 = NULL;
		op2 = NULL;
		op3 = NULL;

		count = countTokenStack(stack);
		change = TRUE;
	}



	if (count == 1)
	{
		op1 = stack->top;

		if (op1->data_type >= TOKEN_NONTERM && op1->data_type <= TOKEN_NONTERM_BOOL)
		{
			if (op1->next_token->token_type == I_STOP)
			{
				tmpitem = op1->next_token;
				op1->next_token = tmpitem->next_token;
				free(tmpitem);

				change = TRUE;
			}
			

			count = countTokenStack(stack);


			// tmpitem = stack->top;
   //  		printf("TOKENY NA STACKU: ");
   //  		while(tmpitem->token_type != I_DOLLAR)
   //  		{
   //  			printf("%d|%d ", tmpitem->data_type, tmpitem->token_type);
   //  			tmpitem = tmpitem->next_token;
   //  		}
   //  		printf("\n");

			if (count == 3)
			{
				result = reduce(stack);
				if (result != SYNTAX_OK)
				{
					return result;
				}
				return SYNTAX_OK;
			}

			if (get_prec_table_index(token_ptr->type) != I_DOLLAR)
			{
				if (get_prec_table_index(token_ptr->type) == I_LEFT_BRACKET)
				{
					// printf("PUSHUJEM: TOKEN_STOP\n");
					result = pushTokenStack(stack, TOKEN_STOP, I_STOP, NULL);
					if (result != SYNTAX_OK)
    				{
    					return result;
    				}
				}

				// printf("PUSHUJEM: %s\n", token_ptr->dynamic_value);
				result = pushTokenStack(stack, token_ptr->type, get_prec_table_index(token_ptr->type), NULL);
				if (result != SYNTAX_OK)
   				{
   					return result;
   				}

				if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
				    fprintf(stderr,"line: %d\n",currentLine);
				    return LEX_ERROR;
				}

				change = TRUE;
			}

			if (change == FALSE)
			{
				// printf("SYNTAX_ERROR\n");
				return SYNTAX_ERROR;
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
				return INTERNAL_ERROR;
			}
		
			new_item->data_type = TOKEN_STOP;
			new_item->token_type = I_STOP;
			new_item->next_token = op2;
			op3->next_token = new_item;
			
			result = shift(stack);

			if(result != SYNTAX_OK)
			{
				// TODO FREE
				return result;
			}


			return SYNTAX_OK;
		}

		gen_rule = test_rule(count, op1, op2, op3);
	}


	else
	{
		// tmpitem = stack->top;
  //   	printf("KONIEC: TOKENY NA STACKU: ");
  //   	while(tmpitem->token_type != I_DOLLAR)
  //   	{
  //   		printf("%d|%d ", tmpitem->data_type, tmpitem->token_type);
  //   		tmpitem = tmpitem->next_token;
  //   	}
		return SYNTAX_ERROR;
	}

	if (gen_rule == NOT_A_RULE)
	{
		return SYNTAX_ERROR;
	}
	else
	{
		char* constant_name = get_name(); 
		if (constant_name == NULL)
		{
			return INTERNAL_ERROR;
		}

		htab_symbol = make_const(constant_name, UNKNOWN);


		result = semantic(op1, op2, op3, htab_symbol, gen_rule, &final_token_type);
		if (result != SYNTAX_OK)
		{
			return result;
		}

		for (int i = 0; i < count; i++)
		{
			popTokenStack(stack);
		}



		result = pushTokenStack(stack, final_token_type, I_DATA, htab_symbol);	// DORIESIT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (result != SYNTAX_OK)
    	{
    		return result;
    	}

    	free(op4);
	}

	return SYNTAX_OK;
}


int semantic(TStackTokenItem op1, TStackTokenItem op2, TStackTokenItem op3, htab_item_t* htab_symbol, Prec_rules_enum rule, TokenTYPE *final_token_type)
{
	// bool op1_to_double = false;
	// bool op3_to_double = false;
	// bool op1_to_integer = false;
	// bool op3_to_integer = false;


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
	
			if (op1->data_type == TOKEN_NONTERM_INT && op3->data_type == TOKEN_NONTERM_DOUBLE)
			{
				*final_token_type = TOKEN_NONTERM_DOUBLE;
				break;
			}

			if (op1->data_type == TOKEN_NONTERM_DOUBLE && op3->data_type == TOKEN_NONTERM_INT)
			{
				*final_token_type = TOKEN_NONTERM_DOUBLE;
				break;
			}

			if (op1->data_type == TOKEN_NONTERM_DOUBLE && op3->data_type == TOKEN_NONTERM_DOUBLE)
			{
				*final_token_type = TOKEN_NONTERM_DOUBLE;
				break;
			}


	
			// if (op1->data_type == TOKEN_NONTERM_INT)
			// 	op1_to_double = true;
	
			// if (op3->data_type == TOKEN_NONTERM_INT)
			// 	op3_to_double = true;
	
			break;
	
		case NT_DIV_NT:
			*final_token_type = TOKEN_NONTERM_DOUBLE;
	
			if (op1->data_type == TOKEN_NONTERM_STRING || op3->data_type == TOKEN_NONTERM_STRING)
				return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			// if (op1->data_type == TOKEN_NONTERM_INT)
			// 	op1_to_double = true;
	
			// if (op3->data_type == TOKEN_NONTERM_INT)
			// 	op3_to_double = true;
	
			break;
	
		case NT_IDIV_NT:
			*final_token_type = TOKEN_NONTERM_INT;
	
			if (op1->data_type == TOKEN_NONTERM_STRING || op3->data_type == TOKEN_NONTERM_STRING)
				return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			// if (op1->data_type == TOKEN_NONTERM_DOUBLE)
			// 	op1_to_integer = true;
	
			// if (op3->data_type == TOKEN_NONTERM_DOUBLE)
			// 	op3_to_integer = true;
	
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
	
			// if (op1->data_type == TOKEN_NONTERM_INT && op3->data_type == TOKEN_NONTERM_DOUBLE)
			// 	op1_to_double = true;
	
			// else if (op1->data_type == TOKEN_NONTERM_DOUBLE && op3->data_type == TOKEN_NONTERM_INT)
			// 	op3_to_double = true;
	
			// else if (op1->data_type != op3->data_type)
			// 	return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
	
			break;
	
		default:
			return SEMANTIC_OTHER_ERROR;
			break;

	}

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
		if (op1->data_type == TOKEN_LEFT_BRACKET && (op2->data_type == TOKEN_NONTERM_INT || op2->data_type == TOKEN_NONTERM_DOUBLE ||op2->data_type == TOKEN_NONTERM_STRING || TOKEN_NONTERM_IDENTIFIER) && op3->data_type == TOKEN_RIGHT_BRACKET)
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

	while((item->token_type != I_DOLLAR && item->token_type != I_STOP) && item->token_type != I_LEFT_BRACKET)
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

int pushTokenStack(TStackToken* stack, TokenTYPE DataType, Prec_table_index_enum TokenType, htab_item_t *htab_symbol)
{
	TStackTokenItem new_item = (TStackTokenItem) malloc(sizeof(struct stacktokenitem)); 
	if (new_item == NULL)
	{
		return INTERNAL_ERROR;
	}

	new_item->data_type = DataType;
	new_item->token_type = TokenType;
	new_item->next_token = stack->top;
	new_item->table_symbol = htab_symbol;

	stack->top = new_item;

	return SYNTAX_OK;
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


char* get_name()
{
	char* name = (char*) malloc(10);
	if (name == NULL)
	{
		return NULL;
	}
	strcpy(name, "%tmp");

	char idx_string[4];
	sprintf(idx_string, "%d", name_counter);

	strcat(name, idx_string);
	name_counter++;

	return name;
}