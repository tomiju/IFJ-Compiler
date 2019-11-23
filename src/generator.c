#include "symtable.h"
#include "generator.h"
#include <stdio.h>
#include "errors.h"
#include <stdlib.h>
#include <stdarg.h>

htab_t* htab_built_in;
htab_t* htab_tf;
tNode* main_func_node;

char* params[] = {"%1", "%2", "%3"};

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

	(*param)->value = htab_instr->value;
	(*param)->key = htab_instr->key;
	(*param)->type = htab_instr->type;
	(*param)->frame = htab_instr->frame;	
	(*param)->isLabel = htab_instr->isLabel;
	(*param)->isConst = htab_instr->isConst;
}

/******************** BLABLA *********************/

void generate_first(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...){
	va_list list;

	tInstr instr;
	InstrInit(&instr, instr_enum);

	va_start(list, count);

	for(unsigned i = 0; i < count; i++){
		htab_item_t* htab_instr = va_arg(list, htab_item_t*);
		
		InstrSetParam(&(instr.param[i]), htab_instr);
	}

	va_end(list);

	InsertLast(instr_list, instr);
	First(instr_list);
}

void generate_instr(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...){
	va_list list;

	tInstr instr;
	InstrInit(&instr, instr_enum);

	va_start(list, count);

	for(unsigned i = 0; i < count; i++){
		htab_item_t* htab_instr = va_arg(list, htab_item_t*);
		
		InstrSetParam(&(instr.param[i]), htab_instr);
	}

	va_end(list);

	PostInsert(instr_list, instr);
	Succ(instr_list);
}

void generate_return_variable(tList* list_instr){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");
	ret_val->frame = LF;
	//htab_item_t* nil = htab_find(htab, "nil");

	generate_instr(list_instr, DEFVAR, 1, ret_val);
	//generate_instr(list_instr, MOVE, 2, item, nil);
}

void generate_func_start(tList* list, htab_item_t* label){
	SetActive(list, main_func_node->prev);

	generate_instr(list, LABEL, 1, label);
	generate_instr(list, PUSHFRAME, 0);

	generate_return_variable(list);
}

/*void generate_def_params(){

}*/

htab_item_t* get_param(unsigned idx){
	htab_item_t* param = htab_find(htab_built_in, params[idx]);
	return param;
}

htab_item_t* get_param_tf(unsigned idx){
	htab_item_t* param = htab_find(htab_tf, params[idx]);
	return param;
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

htab_item_t* generate_var(tList* list, char* name, int type){
	htab_insert(htab_built_in, name, 0, type, LF, false, false, true);
	htab_item_t* var = htab_find(htab_built_in, name);
	generate_instr(list, DEFVAR, 1, var);
	return var;
}

htab_item_t* make_const(int type, int val){
	htab_insert(htab_built_in, "const", val, type, LF, true, false, true);
	return htab_find(htab_built_in, "const");
}

/*void generate_copy_params(tList* list_instr, int count, ...){
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
}*/

/******************** VSTAVANÉ FUNKCIE *********************/

void generate_inputs(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputs");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* string_label = htab_find(htab_built_in, "string");

	generate_func_start(list, func);
	generate_instr(list, READ, 2, retval, string_label);
	generate_func_end(list);
}

void generate_inputf(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputf");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* float_label = htab_find(htab_built_in, "float");

	generate_func_start(list, func);
	generate_instr(list, READ, 2, retval, float_label);
	generate_func_end(list);
}

void generate_inputi(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputi");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* int_label = htab_find(htab_built_in, "int");

	generate_func_start(list, func);
	generate_instr(list, READ, 2, retval, int_label);
	generate_func_end(list);
}

void generate_len(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "len");
	htab_item_t* param1 = get_param(0);
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	
	generate_func_start(list, func);
	generate_instr(list, STRLEN, 2, retval, param1);
	generate_func_end(list);
}

void generate_ord(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "ord");
	htab_item_t* func_len = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	
	generate_func_start(list, func);
	htab_item_t* dlzka = generate_var(list, "dlzka", INT);

	generate_func_call(list, func_len, 1, get_param(0));
	generate_save_return_value(list, dlzka);

	htab_item_t* prava = generate_var(list, "prava", INT);
	htab_item_t* lava = generate_var(list, "lava", INT);

	generate_instr(list, GT, 3, prava, get_param(1), make_const(INT, -1));
	generate_instr(list, LT, 3, lava, get_param(1), dlzka);

	/*htab_item_t* error_label = htab_find(htab_built_in, "error");
	generate_instr(list, JUMPIFNEQ, 3, error_label, prava, lava);*/

	generate_instr(list, STRI2INT, 3, retval, get_param(0), get_param(1));

	generate_func_end(list);
}

void generate_substr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "substr");
	htab_item_t* param1 = get_param(0);
	htab_item_t* param2 = get_param(1);
	htab_item_t* param3 = get_param(2);
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	
	generate_func_start(list, func);


	// TODO


	generate_func_end(list);
}

/*********************** INŠTRUKCIE ************************/

void generator_start(tList* list){
	htab_init(&htab_built_in);
	htab_init(&htab_tf);

	// premenné
	htab_insert(htab_built_in, "%retval", 0, UNKNOWN, LF, false, false, true);
	//htab_insert(htab_built_in, "retval_save", 0, UNKNOWN, TF, false, false, true);
	htab_insert(htab_built_in, params[0], 0, UNKNOWN, LF, false, false, true);
	htab_insert(htab_built_in, params[1], 0, UNKNOWN, LF, false, false, true);
	htab_insert(htab_built_in, params[2], 0, UNKNOWN, LF, false, false, true);
	htab_insert(htab_tf, params[0], 0, UNKNOWN, TF, false, false, true);
	htab_insert(htab_tf, params[1], 0, UNKNOWN, TF, false, false, true);
	htab_insert(htab_tf, params[2], 0, UNKNOWN, TF, false, false, true);

	// konštanta nil
	htab_insert(htab_built_in, "nil", 0, NIL, LF, true, false, true);
	
	// funkcia main
	htab_insert(htab_built_in, "main", 0, FUNC, GF, false, true, true);

	// label error
	htab_insert(htab_built_in, "error", 0, FUNC, GF, false, true, true);

	// dátové tipy
	htab_insert(htab_built_in, "int", 0, TYPE_NAME, GF, false, true, true);
	htab_insert(htab_built_in, "float", 0, TYPE_NAME, GF, false, true, true);
	htab_insert(htab_built_in, "string", 0, TYPE_NAME, GF, false, true, true);

	htab_item_t* main_func = htab_find(htab_built_in, "main");

	generate_first(list, JUMP, 1, main_func);	// skok na main
	generate_instr(list, LABEL, 1, main_func); 	// label na main
	main_func_node = list->last;

	// vstavané funkcie
	generate_inputs(list);
	generate_inputf(list);
	generate_inputi(list);
	generate_len(list);	
	generate_ord(list);
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
					if(instr.param[i]->type == FUNC){
						printf("$");
					}
					printf("%s ", instr.param[i]->key);
				} else if(instr.param[i]->isConst){
					if(instr.param[i]->type != NIL){
						switch(instr.param[i]->type){
							case INT: printf("int@"); break;
							case FLOAT: printf("float@"); break;
							case STRING: printf("string@"); break;
							default: printf("CHYBA_TYPE@"); break;
						};
						
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
						default: printf("CHYBA_FRAME@"); break;
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