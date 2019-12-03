/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   generator.c
 *
 *
 * Datum:    30.11.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */

#include "symtable.h"
#include "generator.h"
#include <stdio.h>
#include "errors.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

htab_t* htab_built_in; // symtable pre vstavané funkcie a
htab_t* htab_tf;	   // symtable pre dočasné parametre na frame

tNode* main_func_node;

tNode* before_while = NULL;

tStack* while_nodes = NULL;
tStack* if_nodes = NULL;
tStack* before_if_jumps = NULL;
tStack* else_nodes = NULL;
tStack* if_else_end_nodes = NULL;

tStackNum* if_num_stack = NULL;

htab_item_t* nil;

htab_item_t* params[256];

unsigned param_idx = 0;
unsigned par_count = 0;

int while_label_idx = -1;

#define NUM_OF_ARGS 3

/************************ MAKRO ***********************/

static const char* INSTR_STRING[] = {
    INSTR(GENERATE_STRING)
};

/********************* LIST INŠTRUKCIÍ *********************/

void InitList (tList* list){
	list->first = NULL;
	list->last = NULL;
	list->active = NULL;
}

void DisposeList (tList* list){
	tNode* act = list->first;
	tNode* del;

	while (act != NULL){
		del = act;
		act = del->next;

		for(int i = 0; i < NUM_OF_ARGS; i++){
			free(del->instr.param[i]);
		}
		free(del);
	}

	list->last = NULL;
	list->first = NULL;
	list->active = NULL;
}

int InsertFirst (tList* list, tInstr instr){
	tNode* new = malloc(sizeof(struct Node));

	if(new == NULL){
		return INTERNAL_ERROR;
	}

	new->instr = instr;
	new->next = list->first;
	new->prev = NULL;

	if (list->first == NULL){
		list->first = new;
		list->last = new;
	} else {
		list->first->prev = new;
		list->first = new;
	}

	return 0;
}

int InsertLast(tList* list, tInstr instr){
	tNode* new = malloc(sizeof(struct Node));

	if(new == NULL){
		return INTERNAL_ERROR;
	}

	new->instr = instr;
	new->next = NULL;
	new->prev = list->last;

	if (list->last == NULL){
		list->first = new;
		list->last = new;
	} else {
		list->last->next = new;
		list->last = new;
	}

	return 0;
}

void First (tList* list){
	list->active = list->first;
}

void Last (tList* list){
	list->active = list->last;
}

void SetActive(tList* list, tNode* node){
	list->active = node;
}

void CopyFirst (tList* list, tInstr *instr){
	if (list->first == NULL){
		return;
	}

	*instr = list->first->instr;
}

void CopyLast (tList* list, tInstr *instr){
	if (list->last == NULL){
		return;
	}

	*instr = list->last->instr;
}

void DeleteFirst (tList* list){
	if (list->first != NULL){
		if (list->active == list->first){
			list->active = NULL;
		}

		list->first = list->first->next;
		free(list->first->prev);
		list->first->prev = NULL;
	}
}

void DeleteLast (tList* list){
	if (list->last){
		if (list->active == list->last){
			list->active = NULL;
		}

		list->last = list->last->prev;
		free(list->last->next);
		list->last->next = NULL;
	}
}

void PostDelete (tList* list){
	if (list->active != NULL && list->active != list->last){
		tNode* del = list->active->next;

		if (del->next != NULL){
			del->next->prev = list->active;
			list->active->next = del->next;
		} else {
			list->active->next = NULL;
			list->last = list->active;
		}

		free(del);
	}
}

void PreDelete (tList* list){
	if (list->active != NULL && list->active != list->first){
		tNode* del = list->active->prev;

		if (del->prev != NULL){
			del->prev->next = list->active;
			list->active->prev = del->prev;
		} else {
			list->active->prev = NULL;
			list->first = list->active;
		}

		free(del);
	}
}

int PostInsert (tList* list, tInstr instr){
	if (list->active != NULL){
		tNode* new = malloc(sizeof(struct Node));

		if(new == NULL){
			return INTERNAL_ERROR;
		}

		new->instr = instr;
		new->next = list->active->next;
		new->prev = list->active;

		if (list->active->next == NULL){
			list->last = new;
		} else {
			list->active->next->prev = new;
		}

		list->active->next = new;
	}

	return 0;
}

int PreInsert (tList* list, tInstr instr){
	if (list->active != NULL){
		tNode* new = malloc(sizeof(struct Node));
		if(new == NULL){
			return INTERNAL_ERROR;
		}

		new->instr = instr;
		new->next = list->active;
		new->prev = list->active->prev;

		if (list->active->prev == NULL){
			list->first = new;
		} else {
			list->active->prev->next = new;
		}

		list->active->prev = new;
	}

	return 0;
}

int Copy (tList* list, tInstr *instr){
	if (list->active == NULL){
		return 1;
	}

	*instr = list->active->instr;
	return 0;
}

void Actualize (tList* list, tInstr instr){
	if (list->active != NULL){
		list->active->instr = instr;
	}
}

void Succ (tList* list){
	if (list->active != NULL){
		list->active = list->active->next;
	}
}

void Pred (tList* list){
	if (list->active != NULL){
		list->active = list->active->prev;
	}
}

int Active (tList* list){
	return(list->active != NULL);
}

void InstrInit(tInstr* instr, enum INSTR_ENUM instr_enum){
	instr->type = instr_enum;
	instr->param[0] = NULL;
	instr->param[1] = NULL;
	instr->param[2] = NULL;
}

void InstrSetParam(tInstrPar** param, htab_item_t* htab_instr){
	(*param) = malloc(sizeof(struct InstrPar));

	switch(htab_instr->type){
		case INT: (*param)->ival = htab_instr->ival; break;
		case FLOAT: (*param)->dval = htab_instr->dval; break;
		case STRING: case BOOL: case NIL: (*param)->sval = htab_instr->sval; break;
	}

	(*param)->key = htab_instr->key;
	(*param)->type = htab_instr->type;
	(*param)->frame = htab_instr->frame;
	(*param)->isLabel = htab_instr->isLabel;
	(*param)->isConst = htab_instr->isConst;
}

/************************** STACK **************************/
void tPushStack(tStack** stack, tNode* node){
	tStack* temp = malloc(sizeof(struct stack));

	temp->node = node;
	temp->link = *stack;

	*stack = temp;
}

tNode* tTopStack(tStack* stack){
	if(stack != NULL){
		return stack->node;
	}
	else{
		return NULL;
	}
}

tNode* tPopStack(tStack** stack){
	tStack* temp;

	if(*stack == NULL){
		return NULL;
	}

	temp = *stack;
	*stack = (*stack)->link;

	tNode* node = temp->node;
	free(temp);
	return node;
}

void tPushStackNum(tStackNum** stack, int num){
	tStackNum* temp = malloc(sizeof(struct stackNum));

	temp->num = num;
	temp->link = *stack;

	*stack = temp;
}

int tTopStackNum(tStackNum* stack){
	return stack->num;
}

int tPopStackNum(tStackNum** stack){
	tStackNum* temp;

	temp = *stack;
	*stack = (*stack)->link;

	int num = temp->num;
	free(temp);
	return num;
}

/*void destroyStack(tStack* stack){
	tStack* temp = malloc(sizeof(struct stack));

    temp = stack;

    while (temp->node != NULL)
    {
        tPopStack(stack);

        temp = temp->link;
    }
}*/

/******************** BLABLA *********************/

htab_item_t* generate_var(tList* list, char* name, int type, int frame){
	htab_item_t* var = htab_find(htab_built_in, name);

	if(var == NULL){
		htab_insert(htab_built_in, name, type, frame, false, false, true);
		var = htab_find(htab_built_in, name);
	} else{
		var->frame = frame;
	}

	generate_instr(list, DEFVAR, 1, var);
	return var;
}

htab_item_t* make_var(char* name, int type, int frame){
	htab_item_t* var = htab_find(htab_built_in, name);

	if(var == NULL){
		htab_insert(htab_built_in, name, type, frame, false, false, true);
		var = htab_find(htab_built_in, name);
	} else{
		var->frame = frame;
	}

	return var;
}

htab_item_t* make_const(char* name, int type){
	htab_item_t* var = htab_find(htab_built_in, name);
	if(var != NULL){
		var->type = type;

		return var;
	} else{
		htab_insert(htab_built_in, name, type, LF, true, false, true);
		return htab_find(htab_built_in, name);
	}
}

htab_item_t* make_label(char* name){
	htab_item_t* var = htab_find(htab_built_in, name);
	if(var != NULL){
		return var;
	} else{
		htab_insert(htab_built_in, name, FUNC, LF, false, true, true);
		var = htab_find(htab_built_in, name);
		var->sval = name;
		return var;
	}
}

void generate_instr_no(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...){

	va_list list;
	htab_item_t** args = malloc(sizeof(htab_item_t*)*3);

	tInstr instr;
	InstrInit(&instr, instr_enum);

	va_start(list, count);
	for(unsigned i = 0; i < count; i++){
		htab_item_t* htab_instr = va_arg(list, htab_item_t*);
		args[i] = htab_instr;

		InstrSetParam(&(instr.param[i]), htab_instr);
	}
	va_end(list);


		if(instr_list->first != NULL){
			PostInsert(instr_list, instr);
			Succ(instr_list);
		} else {
			InsertLast(instr_list, instr);
			First(instr_list);
		}

}

bool check_types(tList* list, enum INSTR_ENUM instr_enum, htab_item_t** args){
	bool converted = false;

	htab_item_t* error_4 = make_const("err4", INT);
	error_4->ival = 4;
	htab_item_t* error_9 = make_const("err9", INT);
	error_9->ival = 9;

	htab_item_t* type_int = make_const("type_int", STRING); //htab_find(htab_built_in, "int");
	type_int->sval = "int";
	htab_item_t* type_float = make_const("type_float", STRING);//htab_find(htab_built_in, "float");
	type_float->sval = "float";

	htab_item_t* const_int_zero = make_const("const_int_zero", INT);
	const_int_zero->ival = 0;
	htab_item_t* const_float_zero = make_const("const_float_zero", FLOAT);
	const_float_zero->dval = 0.0;

	htab_item_t* type1 = htab_find(htab_built_in, "%type1");
	htab_item_t* type2 = htab_find(htab_built_in, "%type2");

	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_convert1 = htab_find(htab_built_in, "%type_converted1");
	htab_item_t* type_convert2 = htab_find(htab_built_in, "%type_converted2");

	switch(instr_enum){
		case LT: case GT: case EQ:
			if(args[1]->type == args[2]->type){
				break;
			} else if(args[1]->type == NIL || args[2]->type == NIL){	// at least one is NIL
				if(instr_enum != EQ){		// NOT EQ => can't be NIL => error
					generate_instr_no(list, EXIT, 1, error_4);
				}
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){	// at least one is UNKNOWN
				converted = true;
			
				start_if_else(list);
				generate_instr_no(list, TYPE, 2, type1, args[1]);
				generate_instr_no(list, TYPE, 2, type2, args[2]);
				generate_instr_no(list, EQ, 3, type_control, type1, type2);		
				generate_condition_check(list, type_control, false);
				generate_if(list);

					generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]);

				generate_else(list);

					start_if_else(list);
					generate_instr_no(list, EQ, 3, type_control, type1, type_int);
					generate_condition_check(list, type_control, false);
					generate_if(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type2, type_float);
						generate_condition_check(list, type_control, false);
						generate_if(list);

							generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
							generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					generate_else(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type1, type_float);
						generate_condition_check(list, type_control, false);
						generate_if(list);

							start_if_else(list);
							generate_instr_no(list, EQ, 3, type_control, type2, type_int);
							generate_condition_check(list, type_control, false);
							generate_if(list);

								generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]);
								generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1);

							generate_else(list);

								generate_instr(list, EXIT, 1, error_4);

							end_if_else(list);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			} else if(args[1]->type == INT && args[2]->type == FLOAT){
				converted = true;
				generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
				generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]);
			} else if(args[1]->type == FLOAT && args[2]->type == INT){
				converted = true;
				generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]);
				generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1);
			} else {
				generate_instr_no(list, EXIT, 1, error_4);
			}
			break; 
		
		case ADD: case SUB: case MUL:
			if(args[1]->type == INT && args[2]->type == INT){
				break;
			} else if(args[1]->type == FLOAT && args[2]->type == FLOAT){
				break;
			} else if(args[1]->type == NIL || args[2]->type == NIL){	// at least one is NIL
				generate_instr_no(list, EXIT, 1, error_4);
			} else if(args[1]->type == BOOL || args[2]->type == BOOL){	// at least one is NIL
				generate_instr_no(list, EXIT, 1, error_4);
			} else if(args[1]->type == STRING || args[2]->type == STRING){	// at least one is NIL
				generate_instr_no(list, EXIT, 1, error_4);
			} else if(args[1]->type == INT && args[2]->type == FLOAT){
				converted = true;
				generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
				generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]);
			} else if(args[1]->type == FLOAT && args[2] == INT){
				converted = true;
				generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]);
				generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1);
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){
				converted = true;
			
				start_if_else(list);
				generate_instr_no(list, TYPE, 2, type1, args[1]);
				generate_instr_no(list, TYPE, 2, type2, args[2]);
				generate_instr_no(list, EQ, 3, type_control, type1, type2);		
				generate_condition_check(list, type_control, false);
				generate_if(list);

					start_if_else(list);
					generate_instr_no(list, EQ, 3, type_control, type1, type_int);		
					generate_condition_check(list, type_control, false);
					generate_if(list);

						generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]);

					generate_else(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type1, type_float);		
						generate_condition_check(list, type_control, false);
						generate_if(list);

							generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					end_if_else(list);

				generate_else(list);

					start_if_else(list);
					generate_instr_no(list, EQ, 3, type_control, type1, type_int);
					generate_condition_check(list, type_control, false);
					generate_if(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type2, type_float);
						generate_condition_check(list, type_control, false);
						generate_if(list);

							generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
							generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					generate_else(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type1, type_float);
						generate_condition_check(list, type_control, false);
						generate_if(list);

							start_if_else(list);
							generate_instr_no(list, EQ, 3, type_control, type2, type_int);
							generate_condition_check(list, type_control, false);
							generate_if(list);

								generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]);
								generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1);

							generate_else(list);

								generate_instr_no(list, EXIT, 1, error_4);

							end_if_else(list);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			} else {
				generate_instr_no(list, EXIT, 1, error_4);
			}
			break;
		
		case DIV: 
			if(args[1]->type == FLOAT && args[2]->type == FLOAT){
				if(args[2]->dval == 0.0){
					generate_instr_no(list, EXIT, 1, error_9);
				} else {
					break;
				}
			} else if(args[1]->type == INT && args[2]->type == FLOAT){
				converted = true;

				if(args[2]->dval == 0.0){
					generate_instr_no(list, EXIT, 1, error_9);
				} else {
					generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
					generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]);
				}
			} else if(args[1]->type == FLOAT && args[2]->type == INT){
				converted = true;

				if(args[2]->ival == 0){
					generate_instr_no(list, EXIT, 1, error_9);		
				} else {
					generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]);
					generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1);
				}
			} else if(args[1]->type == INT && args[2]->type == INT){
				converted = true;

				if(args[2]->ival == 0){
					generate_instr_no(list, EXIT, 1, error_9);		
				} else {
					generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
					generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]);
					generate_instr_no(list, instr_enum, 3, args[0], type_convert1, type_convert2);
				}
				
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){
				converted = true;
			
				start_if_else(list);
				generate_instr_no(list, TYPE, 2, type1, args[1]);
				generate_instr_no(list, TYPE, 2, type2, args[2]);
				generate_instr_no(list, EQ, 3, type_control, type1, type2);		
				generate_condition_check(list, type_control, false);
				generate_if(list);

					start_if_else(list);
					generate_instr_no(list, EQ, 3, type_control, type1, type_int);		
					generate_condition_check(list, type_control, false);
					generate_if(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero);		
						generate_condition_check(list, type_control, false);
						generate_if(list);

							generate_instr_no(list, EXIT, 1, error_9);		

						generate_else(list);

							generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
							generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]);
							generate_instr_no(list, instr_enum, 3, args[0], type_convert1, type_convert2);

						end_if_else(list);

					generate_else(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type1, type_float);		
						generate_condition_check(list, type_control, false);
						generate_if(list);

							start_if_else(list);
							generate_instr_no(list, EQ, 3, type_control, args[2], const_float_zero);		
							generate_condition_check(list, type_control, false);
							generate_if(list);

								generate_instr_no(list, EXIT, 1, error_9);
								
							generate_else(list);

								generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]);
							
							end_if_else(list);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					end_if_else(list);
				generate_else(list);

					start_if_else(list);
					generate_instr_no(list, EQ, 3, type_control, type1, type_float);
					generate_condition_check(list, type_control, false);
					generate_if(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type2, type_int);
						generate_condition_check(list, type_control, false);
						generate_if(list);

							start_if_else(list);
							generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero);		
							generate_condition_check(list, type_control, false);
							generate_if(list);

								generate_instr_no(list, EXIT, 1, error_9);		

							generate_else(list);

								generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]);
								generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1);

							end_if_else(list);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);
							
						end_if_else(list);

					generate_else(list);

						start_if_else(list);
						generate_instr_no(list, EQ, 3, type_control, type1, type_int);
						generate_condition_check(list, type_control, false);
						generate_if(list);

							start_if_else(list);
							generate_instr_no(list, EQ, 3, type_control, type2, type_float);
							generate_if(list);

								start_if_else(list);
								generate_instr_no(list, EQ, 3, type_control, args[2], const_float_zero);		
								generate_condition_check(list, type_control, false);
								generate_if(list);

									generate_instr_no(list, EXIT, 1, error_9);		

								generate_else(list);

									generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]);
									generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]);

								end_if_else(list);

							generate_else(list);

								generate_instr_no(list, EXIT, 1, error_4);		

							end_if_else(list);

						generate_else(list);

							generate_instr_no(list, EXIT, 1, error_4);

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			} else {
				generate_instr_no(list, EXIT, 1, error_4);
			}
			break;
		
		default: break;
	}

	return converted;
}

void generate_instr(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...){

	va_list list;
	htab_item_t** args = malloc(sizeof(htab_item_t*)*3);

	tInstr instr;
	InstrInit(&instr, instr_enum);

	va_start(list, count);
	for(unsigned i = 0; i < count; i++){
		htab_item_t* htab_instr = va_arg(list, htab_item_t*);
		args[i] = htab_instr;

		InstrSetParam(&(instr.param[i]), htab_instr);
	}
	va_end(list);

	if(!check_types(instr_list, instr_enum, args)){
		if(instr_list->first != NULL){
			PostInsert(instr_list, instr);
			Succ(instr_list);
		} else {
			InsertLast(instr_list, instr);
			First(instr_list);
		}
	}
}

htab_item_t* get_param(unsigned idx){
	char arg_string[5] = "%";

	char idx_string[4];
	sprintf(idx_string, "%d", idx);

	strcat(arg_string, idx_string);

	htab_item_t* param = htab_find(htab_built_in, arg_string);
	if(param == NULL){
		htab_insert(htab_built_in, arg_string, UNKNOWN, LF, false, false, true);
		param = htab_find(htab_built_in, arg_string);
		//param->ival = 0;
	}

	return param;
}

htab_item_t* get_while_end(){
	char label_string[13] = "while_end";

	char idx_string[4];
	sprintf(idx_string, "%d", while_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_while_label(){
	char label_string[9] = "while";

	char idx_string[4];
	sprintf(idx_string, "%d", while_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_if_end(){
	char label_string[11] = "if_end";

	char idx_string[4];
	sprintf(idx_string, "%d", tTopStackNum(if_num_stack));

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_if_label(){
	char label_string[7] = "if";

	char idx_string[4];
	sprintf(idx_string, "%d", tTopStackNum(if_num_stack));

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_else_label(){
	char label_string[9] = "else";

	char idx_string[4];
	sprintf(idx_string, "%d", tTopStackNum(if_num_stack));

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_param_tf(unsigned idx){
	char arg_string[4] = "%";

	char idx_string[4];
	sprintf(idx_string, "%d", idx);

	strcat(arg_string, idx_string);

	htab_item_t* param = htab_find(htab_tf, arg_string);
	if(param == NULL){
		htab_insert(htab_tf, arg_string, UNKNOWN, TF, false, false, true);
		param = htab_find(htab_tf, arg_string);
	}

	return param;
}

void start_if_else(tList* list){
	static int if_label_idx = 0;
	if_label_idx++;
	tPushStackNum(&if_num_stack, if_label_idx);

	htab_item_t* true_const = make_const("true", BOOL);
	true_const->sval = "true";

	tNode* if_condition = list->active;

	generate_instr(list, LABEL, 1, get_if_label());

	tPushStack(&if_nodes, list->active);

	generate_instr(list, JUMP, 1, get_if_end());
	generate_instr(list, LABEL, 1, get_else_label());

	tPushStack(&else_nodes, list->active);

	generate_instr(list, LABEL, 1, get_if_end());

	tPushStack(&if_else_end_nodes, list->active);

	SetActive(list, if_condition);
}

void generate_condition_check(tList* list, htab_item_t* podmienka, bool isWhile){
	htab_item_t* con_true = make_const("const_true", BOOL);
	con_true->sval = "true";

	if(isWhile){
		htab_item_t* label_while_end = get_while_end();

		generate_instr(list, JUMPIFNEQ, 3, label_while_end, podmienka, con_true);
	} else {
		//htab_item_t* label_if = get_if_label();
		htab_item_t* label_else = get_else_label();

		//int if_label = tPopStackNum(&if_num_stack);
		htab_item_t* label_if = get_if_label();

		tPushStack(&before_if_jumps, list->active);

		generate_instr(list, JUMPIFEQ, 3, label_if, podmienka, con_true);
		generate_instr(list, JUMP, 1, label_else);
	}
}

void generate_if(tList* list){
	SetActive(list, tPopStack(&if_nodes));
}

void generate_else(tList* list){
	SetActive(list, tPopStack(&else_nodes));
}

void end_if_else(tList* list){
	SetActive(list, tPopStack(&if_else_end_nodes));
	tPopStackNum(&if_num_stack);
	tPopStack(&before_if_jumps);
}

void generate_before_whiles(tList* list, htab_item_t* item){
	tNode* temp = list->active;

	if(before_while != NULL){
		SetActive(list, before_while);
	}

	generate_instr(list, DEFVAR, 1, item);

	if(before_while != NULL){
		SetActive(list, temp);
		before_while = before_while->next;
	}
}

void generate_before_if(tList* list, htab_item_t* item){
	tNode* temp = list->active;
	tNode* before_jump = tTopStack(before_if_jumps);

	if(before_jump != NULL){
		SetActive(list, before_jump);
		generate_instr(list, DEFVAR, 1, item);
		SetActive(list, temp);
	} else {
		generate_instr(list, DEFVAR, 1, item);
	}
}


void generate_while_start(tList* list){
	while_label_idx++;

	if(before_while == NULL){
		before_while = list->active;
	}

	generate_instr(list, LABEL, 1, get_while_label());

	tNode* node = list->active;

	generate_instr(list, JUMP, 1, get_while_label());
	generate_instr(list, LABEL, 1, get_while_end());

	tPushStack(&while_nodes, list->active);

	SetActive(list, node);
}

void generate_while_end(tList* list){
	SetActive(list, tPopStack(&while_nodes));

	if(while_nodes == NULL){
		before_while = NULL;
	}
}

void generate_return_variable(tList* list_instr){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");
	ret_val->frame = LF;
	//htab_item_t* nil = htab_find(htab, "nil");

	generate_instr(list_instr, DEFVAR, 1, ret_val);
	generate_instr(list_instr, MOVE, 2, ret_val, nil);
}

void generate_func_start(tList* list, htab_item_t* label){
	SetActive(list, main_func_node->prev);

	generate_instr(list, LABEL, 1, label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list);
}

void generate_save_to_return(tList* list, htab_item_t* value_to_save){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");

	generate_instr(list, MOVE, 2, ret_val, value_to_save);
}

void generate_return(tList* list){
	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);
}

void generate_func_end(tList* list){
	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);

	Last(list);
}

void generate_save_return_value(tList* list, htab_item_t* var){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");
	ret_val->frame = TF;

	generate_instr(list, MOVE, 2, var, ret_val);
	ret_val->frame = LF;
}

void send_param(htab_item_t* par){
	params[param_idx++] = par;
}

void func_call(tList* list, htab_item_t* func){
	htab_item_t* param;
	htab_item_t* val_to_copy;

	htab_item_t* space_const = make_const("medzera", STRING);
	space_const->sval = " ";

	htab_item_t* new_line_const = make_const("odriadkovanies", STRING);
	new_line_const->sval = "\n";

	if(strcmp(func->key, "print") == 0){
		for(unsigned i = 0; i < param_idx; i++){
			generate_instr(list, CREATEFRAME, 0);

			val_to_copy = params[i];
			param = get_param_tf(0);

			generate_instr(list, DEFVAR, 1, param);
			generate_instr(list, MOVE, 2, param, val_to_copy);

			generate_instr(list, CALL, 1, func);

			generate_instr(list, CREATEFRAME, 0);

			if(param_idx != (i + 1)){
				generate_instr(list, DEFVAR, 1, param);
				generate_instr(list, MOVE, 2, param, space_const);
			}else {
				generate_instr(list, DEFVAR, 1, param);
				generate_instr(list, MOVE, 2, param, new_line_const);
			}

			generate_instr(list, CALL, 1, func);
		}
		par_count = 0;
		param_idx = 0;
	} else {
		generate_instr(list, CREATEFRAME, 0);

		for(unsigned i = 0; i < param_idx; i++){
			val_to_copy = params[i];
			param = get_param_tf(i);

			generate_instr(list, DEFVAR, 1, param);
			generate_instr(list, MOVE, 2, param, val_to_copy);
		}

		generate_instr(list, CALL, 1, func);

		par_count = param_idx;
		param_idx = 0;
	}
}

void generate_func_call(tList* list, htab_item_t* label, unsigned count, ...){
	va_list arg_list;

	htab_item_t* param;
	htab_item_t* val_to_copy;

	generate_instr(list, CREATEFRAME, 0);

	va_start(arg_list, count);

	for(unsigned i = 0; i < count; i++){
		val_to_copy = va_arg(arg_list, htab_item_t*);
		param = get_param_tf(i);

		generate_instr(list, DEFVAR, 1, param);
		generate_instr(list, MOVE, 2, param, val_to_copy);
	}

	va_end(arg_list);

	generate_instr(list, CALL, 1, label);
}

/******************** VSTAVANÉ FUNKCIE *********************/

void generate_inputs(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputs");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* string_label = htab_find(htab_built_in, "string");

	generate_func_start(list, func);
	generate_instr_no(list, READ, 2, retval, string_label);
	generate_func_end(list);
}

void generate_inputf(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputf");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* float_label = htab_find(htab_built_in, "float");

	generate_func_start(list, func);
	generate_instr_no(list, READ, 2, retval, float_label);
	generate_func_end(list);
}

void generate_inputi(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputi");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* int_label = htab_find(htab_built_in, "int");

	generate_func_start(list, func);
	generate_instr_no(list, READ, 2, retval, int_label);
	generate_func_end(list);
}

void generate_len(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	/*htab_item_t* type_check = htab_find(htab_built_in, "%type_control");
	htab_item_t* nil = htab_find(htab_built_in, "nil");
	htab_item_t* error_4 = htab_find(htab_built_in, "err4");*/

	generate_func_start(list, func);

	/*start_if_else(list);
	generate_instr_no(list, EQ, 3, type_check, get_param(0), nil);
	generate_condition_check(list, type_check, false);
	generate_if(list);

	generate_instr_no(list, EXIT, 1, error_4);

	generate_else(list);*/

	generate_instr_no(list, STRLEN, 2, retval, get_param(0));
	
	//end_if_else(list);
	generate_func_end(list);
}

void generate_ord(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "ord");
	htab_item_t* func_len = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	generate_func_start(list, func);
	htab_item_t* dlzka = generate_var(list, "dlzka", INT, LF);

	generate_func_call(list, func_len, 1, get_param(0));
	generate_save_return_value(list, dlzka);

	htab_item_t* prava = generate_var(list, "prava", INT, LF);
	htab_item_t* lava = generate_var(list, "lava", INT, LF);

	htab_item_t* con = make_const("minus_one" ,INT);
	con->ival = -1;
	generate_instr_no(list, GT, 3, prava, get_param(1), con);
	generate_instr_no(list, LT, 3, lava, get_param(1), dlzka);

	/*htab_item_t* error_label = htab_find(htab_built_in, "error");
	generate_instr(list, JUMPIFNEQ, 3, error_label, prava, lava);*/

	generate_instr_no(list, STRI2INT, 3, retval, get_param(0), get_param(1));

	generate_func_end(list);
}

void generate_chr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "chr");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	generate_func_start(list, func);
	generate_instr_no(list, INT2CHAR, 2, retval, get_param(0));
	generate_func_end(list);
}

void generate_substr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "substr");
	htab_item_t* func_len = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	generate_func_start(list, func);

	htab_item_t* con = make_const("None", STRING);
	con->sval = "";
	generate_instr_no(list, MOVE, 2, retval, con); // TODO None STRING

	htab_item_t* dlzka = generate_var(list, "dlzka", INT, LF);
	generate_func_call(list, func_len, 1, get_param(0));

	generate_save_return_value(list, dlzka);

	htab_item_t* podmienky = generate_var(list, "podmienky", BOOL, LF);

	htab_item_t* con_zero = make_const("zero", INT);
	con_zero->ival = 0;

	htab_item_t* con_one = make_const("one", INT);
	con_one->ival = 1;

	htab_item_t* con_true = make_const("const_true", BOOL);
	con_true->sval = "true";
	generate_instr_no(list, LT, 3, podmienky, dlzka, con_zero);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true );

	generate_instr_no(list, LT, 3, podmienky, get_param(1), con_zero);
	generate_instr_no(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	generate_instr_no(list, EQ, 3, podmienky, get_param(1), dlzka);
	generate_instr_no(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	generate_instr_no(list, GT, 3, podmienky, get_param(1), dlzka);
	generate_instr_no(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	generate_instr_no(list, EQ, 3, podmienky, get_param(2), con_zero);
	generate_instr_no(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	htab_item_t* ch = generate_var(list, "char", STRING, LF);
	htab_item_t* idx = generate_var(list, "index", INT, LF);
	generate_instr_no(list, MOVE, 2, idx, get_param(1));
	htab_item_t* last_idx = generate_var(list, "last_idx", INT, LF);
	generate_instr_no(list, ADD, 3, last_idx, get_param(1), get_param(2));

	htab_item_t* loop_label = make_label("loop");
	htab_item_t* end_label = make_label("end");

	generate_instr_no(list, LABEL, 1, loop_label);
	generate_instr_no(list, GETCHAR, 3, ch, get_param(0), idx);
	generate_instr_no(list, CONCAT, 3, retval, retval, ch);
	generate_instr_no(list, ADD, 3, idx, idx, con_one);
	generate_instr_no(list, EQ, 3, podmienky, idx, last_idx);
	generate_instr_no(list, JUMPIFEQ, 3, end_label, podmienky, con_true);
	generate_instr_no(list, LT, 3, podmienky, idx, dlzka);
	generate_instr_no(list, JUMPIFEQ, 3, loop_label, podmienky, con_true);

	generate_instr_no(list, LABEL, 1, end_label);

	generate_func_end(list);
}

void generate_print(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "print");

	generate_func_start(list, func);

	htab_item_t* param = get_param(0);

	generate_instr_no(list, WRITE, 1, param);

	generate_func_end(list);
}

/*********************** INŠTRUKCIE ************************/

void generator_start(tList* list){
	htab_init(&htab_built_in);
	htab_init(&htab_tf);

	// premenné
	htab_insert(htab_built_in, "%retval", UNKNOWN, LF, false, false, true);
	// konštanta nil
	htab_insert(htab_built_in, "nil", NIL, LF, true, false, true);
	htab_item_t* item = htab_find(htab_built_in, "nil");
	nil = item;
	nil->sval = "nil";

	// funkcia main
	htab_insert(htab_built_in, "$main", FUNC, GF, false, true, true);
	item = htab_find(htab_built_in, "$main");
	item->sval = "$main";

	// dátové tipy
	htab_insert(htab_built_in, "int", TYPE_NAME, GF, false, true, true);
	item = htab_find(htab_built_in, "int");
	item->sval = "int";
	htab_insert(htab_built_in, "float", TYPE_NAME, GF, false, true, true);
	item = htab_find(htab_built_in, "float");
	item->sval = "float";
	htab_insert(htab_built_in, "string", TYPE_NAME, GF, false, true, true);
	item = htab_find(htab_built_in, "string");
	item->sval = "string";

	htab_item_t* main_func = htab_find(htab_built_in, "$main");

	generate_instr_no(list, JUMP, 1, main_func);	// skok na main
	generate_instr_no(list, LABEL, 1, main_func); 	// label na main
	main_func_node = list->last;

	// pomocné premenné na kontrolu typov
	generate_var(list, "%type_control", BOOL, GF);
	generate_var(list, "%type_converted1", FLOAT, GF);
	generate_var(list, "%type_converted2", FLOAT, GF);
	generate_var(list, "%type1", UNKNOWN, GF);
	generate_var(list, "%type2", UNKNOWN, GF);

	// vstavané funkcie
	generate_len(list);
	generate_ord(list);
	generate_chr(list);
	generate_substr(list);
	generate_inputs(list);
	generate_inputf(list);
	generate_inputi(list);
	generate_print(list);
}

char* replace_by_escape(char* string){
	char* replaced = malloc((strlen(string)+1)*4);

	unsigned rep_idx = 0;
	for(unsigned str_idx = 0; str_idx < strlen(string); str_idx++, rep_idx++){
		switch(string[str_idx]){
		    case ' ':
		    	replaced[rep_idx++] = '\\';
		    	replaced[rep_idx++] = '0';
			    replaced[rep_idx++] = '3';
			    replaced[rep_idx] = '2';
			    break;
			case '\t':
				replaced[rep_idx++] = '\\';
			    replaced[rep_idx++] = '0';
			    replaced[rep_idx++] = '0';
			    replaced[rep_idx] = '9';
			    break;
			case '\n':
				replaced[rep_idx++] = '\\';
			    replaced[rep_idx++] = '0';
			    replaced[rep_idx++] = '1';
			    replaced[rep_idx] = '0';
			    break;
			case '\\':
				replaced[rep_idx++] = '\\';
			    replaced[rep_idx++] = '0';
			    replaced[rep_idx++] = '9';
			    replaced[rep_idx] = '2';
			    break;
			case '#':
				replaced[rep_idx++] = '\\';
			    replaced[rep_idx++] = '0';
			    replaced[rep_idx++] = '3';
			    replaced[rep_idx] = '5';
			    break;
		    default: replaced[rep_idx] = string[str_idx];
		}
	}
	replaced[rep_idx] = '\0';
	return replaced;
}

void printInstructions(tList* list){
	printf(".IFJcode19\n");

	First(list);
	tInstr instr;

	while(Active(list)){
		Copy(list, &instr); 

		char* repl;

		printf("%s ", INSTR_STRING[instr.type]); 

		for(int i = 0; i < 3; i++){
			if(instr.param[i] != NULL){
				if(instr.param[i]->isLabel){
					if(instr.param[i]->type == FUNC){
						printf("$");
					}
					printf("%s ", instr.param[i]->key);
				} else if(instr.param[i]->isConst){
					switch(instr.param[i]->type){
						case INT: printf("int@%d", instr.param[i]->ival); break;
						case FLOAT: printf("float@%a", instr.param[i]->dval); break;
						case STRING: 
							repl = replace_by_escape(instr.param[i]->sval);
							printf("string@%s", repl);
							free(repl);
							break;
						case BOOL: printf("bool@%s", instr.param[i]->sval); break;
						case NIL: printf("nil@%s", instr.param[i]->sval); break;
					};
				} else {
					switch(instr.param[i]->frame){
						case GF: printf("GF@"); break;
						case LF: printf("LF@"); break;
						case TF: printf("TF@"); break;
					}
					printf("%s ", instr.param[i]->key);
				}
			}
		}

		printf("\n");
		Succ(list);
	}

	DisposeList(list);
	htab_clear(htab_built_in);
	htab_clear(htab_tf);
}