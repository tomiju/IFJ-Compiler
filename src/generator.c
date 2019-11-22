#include "symtable.h"
#include "generator.h"
#include <stdio.h>
#include "errors.h"
#include <stdlib.h>
#include <stdarg.h>

htab_t* htab_built_in;

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

/******************** BLABLA *********************/

void generate_instr(tList* instr_list, enum INSTR_ENUM instr_enum, int count, ...){
	va_list list;

	tInstr instr;
	instr.type = instr_enum;
	instr.param[0] = NULL;
	instr.param[1] = NULL;
	instr.param[2] = NULL;

	va_start(list, count);

	for(int i = 0; i < count; i++){
		instr.param[i] = va_arg(list, htab_item_t*);
	}

	va_end(list);

	InsertLast(instr_list, instr);
}

void generate_return_variable(tList* list_instr, htab_t* htab){
	htab_item_t* item = htab_find(htab, "retval");
	htab_item_t* nil = htab_find(htab, "nil");

	generate_instr(list_instr, DEFVAR, 1, item);
	generate_instr(list_instr, MOVE, 2, item, nil);
}

void generate_copy_params(tList* list_instr, int count, ...){
	va_list list;
	htab_item_t* loc_var;
	htab_item_t* param;

	va_start(list, count);
	for(int i = 0; i < count; i++){
		loc_var = va_arg(list, htab_item_t*);
		param = va_arg(list, htab_item_t*);

		generate_instr(list_instr, DEFVAR, 1, loc_var);
		generate_instr(list_instr, MOVE, 2, loc_var, param);
	}

	va_end(list);
}

/******************** VSTAVANÉ FUNKCIE *********************/

void generate_inputs(tList* list, htab_t* htab){
	htab_item_t* func_label = htab_find(htab, "inputs");
	htab_item_t* retval = htab_find(htab, "retval");
	htab_item_t* string_label = htab_find(htab, "string");

	generate_instr(list, LABEL, 1, func_label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list, htab);

	generate_instr(list, READ, 2, retval, string_label);

	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);
}

void generate_inputf(tList* list, htab_t* htab){
	htab_item_t* func_label = htab_find(htab, "inputf");
	htab_item_t* retval = htab_find(htab, "retval");
	htab_item_t* float_label = htab_find(htab, "float");

	generate_instr(list, LABEL, 1, func_label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list, htab);

	generate_instr(list, READ, 2, retval, float_label);

	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);
}

void generate_inputi(tList* list, htab_t* htab){
	htab_item_t* func_label = htab_find(htab, "inputi");
	htab_item_t* retval = htab_find(htab, "retval");
	htab_item_t* int_label = htab_find(htab, "int");

	generate_instr(list, LABEL, 1, func_label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list, htab);

	generate_instr(list, READ, 2, retval, int_label);

	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);
}

void generate_len(tList* list, htab_t* htab){
	htab_item_t* func_label = htab_find(htab, "len");
	htab_item_t* loc_var1 = htab_find(htab, "loc_var1");
	htab_item_t* param1 = htab_find(htab, "%1");
	htab_item_t* retval = htab_find(htab, "retval");

	param1->frame = LF;
	
	generate_instr(list, LABEL, 1, func_label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list, htab);
	generate_copy_params(list, 1, loc_var1, param1);

	generate_instr(list, STRLEN, 2, retval, loc_var1);

	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);
}

void generate_substr(tList* list, htab_t* htab){
	htab_item_t* func_label = htab_find(htab, "substr");
	htab_item_t* loc_var1 = htab_find(htab, "loc_var1");
	htab_item_t* loc_var2 = htab_find(htab, "loc_var2");
	htab_item_t* loc_var3 = htab_find(htab, "loc_var3");
	htab_item_t* param1 = htab_find(htab, "%1");
	htab_item_t* param2 = htab_find(htab, "%5");
	htab_item_t* param3 = htab_find(htab, "%3");
	htab_item_t* retval = htab_find(htab, "retval");

	generate_instr(list, LABEL, 1, func_label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list, htab);
	generate_copy_params(list, 3, loc_var1, param1, loc_var2, param2, loc_var3, param3);


	// TODO


	generate_instr(list, POPFRAME, 0);
	generate_instr(list, RETURN, 0);
}

/*********************** INŠTRUKCIE ************************/

void generator_start(tList* list){
	htab_init(&htab_built_in);

	htab_insert(htab_built_in, "retval", 0, UNKNOWN, LF, false, false, true);
	htab_insert(htab_built_in, "loc_var1", 0, UNKNOWN, LF, false, false, true);
	htab_insert(htab_built_in, "%1", 0, UNKNOWN, LF, false, false, true);
	htab_insert(htab_built_in, "nil", 0, NIL, LF, true, false, true);
	htab_insert(htab_built_in, "main", 0, FUNC, GF, false, true, true);

	htab_insert(htab_built_in, "int", 0, FUNC, GF, false, true, true);
	htab_insert(htab_built_in, "float", 0, FUNC, GF, false, true, true);
	htab_insert(htab_built_in, "string", 0, FUNC, GF, false, true, true);

	htab_item_t* main_func = htab_find(htab_built_in, "main");

	generate_instr(list, JUMP, 1, main_func);

	generate_inputs(list, htab_built_in);
	generate_inputf(list, htab_built_in);
	generate_inputi(list, htab_built_in);

	generate_len(list, htab_built_in);	

	generate_instr(list, LABEL, 1, main_func);
}

void printInstructions(tList* list){
	printf(".IFJcode19\n");

	First(list);
	tInstr instr;

	while(Active(list)){
		Copy(list, &instr); 
		printf("%s ", INSTR_STRING[instr.type]); 

		for(int i = 0; i < 3; i++){
			if(instr.param[i] != NULL){
				if(instr.param[i]->isLabel){
					printf("$%s ", instr.param[i]->key);
				} else if(instr.param[i]->isConst){
					if(instr.param[i]->type != NIL){
						switch(instr.param[i]->frame){
							case GF: printf("GF@"); break;
							case LF: printf("LF@"); break;
							case TF: printf("TF@"); break;
						}
						
						printf("%d ", instr.param[i]->value);
					/*} else if(instr.param[i]->type == DOUBLE) {
						printf("%s@%lf ", instr.param[i]->frame, instr.param[i]->value);
					} else if(instr.param[i]->type == STRING) {
						printf("%s@%s ", instr.param[i]->frame, instr.param[i]->value);
					*/} else {
						printf("nil@nil ");
					}
				} else {
					//if(instr.param[i]->type != NIL){
						switch(instr.param[i]->frame){
							case GF: printf("GF@"); break;
							case LF: printf("LF@"); break;
							case TF: printf("TF@"); break;
						}
						printf("%s ", instr.param[i]->key);
					/*} else {
						printf("%s ", "TODO");
					}*/
				}
			}
		}

		printf("\n");
		Succ(list);
	}
}