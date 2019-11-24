#include "symtable.h"
#include "generator.h"
#include <stdio.h>
#include "errors.h"
#include <stdlib.h>
#include <stdarg.h>

htab_t* htab_built_in;
htab_t* htab_tf;
tNode* main_func_node;

#define NUM_OF_ARGS 3

char* str_true = "true";
int zero_0 = 0;
int one_1 = 1;
int minus_one = -1; 

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
		case INT: case FUNC: (*param)->value.ival = htab_instr->value.ival; break;
		case FLOAT: (*param)->value.dval = htab_instr->value.dval; break;
		case STRING: case NIL: case BOOL: case TYPE_NAME: (*param)->value.sval = htab_instr->value.sval; break;
	}
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

htab_item_t* get_param(unsigned idx){
	char arg_string[4] = "%";

	char idx_string[4];
	sprintf(idx_string, "%d", idx);

	strcat(arg_string, idx_string);

	htab_item_t* param = htab_find(htab_built_in, arg_string);
	if(param == NULL){
		htab_insert(htab_built_in, arg_string, &zero_0, UNKNOWN, LF, false, false, true);
		param = htab_find(htab_built_in, arg_string);
	}

	return param;
}

htab_item_t* get_param_tf(unsigned idx){
	char arg_string[4] = "%";

	char idx_string[4];
	sprintf(idx_string, "%d", idx);

	strcat(arg_string, idx_string);

	htab_item_t* param = htab_find(htab_tf, arg_string);
	if(param == NULL){
		htab_insert(htab_tf, arg_string, &zero_0, UNKNOWN, TF, false, false, true);
		param = htab_find(htab_tf, arg_string);
	}

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
	htab_item_t* var = htab_find(htab_built_in, name);

	if(var == NULL){
		htab_insert(htab_built_in, name, &zero_0, type, LF, false, false, true);
		var = htab_find(htab_built_in, name);
	}
	
	generate_instr(list, DEFVAR, 1, var);
	return var;
}

htab_item_t* make_const(char* name, int type, void* val){
	htab_item_t* var = htab_find(htab_built_in, name);
	if(var != NULL){
		var->type = type;

		switch(var->type){
			case INT: case FUNC: var->value.ival = val; break;
			case FLOAT: var->value.dval = val; break;
			case STRING: case NIL: case BOOL: case TYPE_NAME: var->value.sval = val; break;
		}

		return var;
	} else{
		htab_insert(htab_built_in, name, val, type, LF, true, false, true);
		return htab_find(htab_built_in, name);
	}
}

htab_item_t* make_label(char* name){
	htab_item_t* var = htab_find(htab_built_in, name);
	if(var != NULL){
		return var;
	} else{
		htab_insert(htab_built_in, name, name, FUNC, LF, false, true, true);
		return htab_find(htab_built_in, name);
	}
}

/******************** VSTAVANÉ FUNKCIE *********************/

void generate_inputs(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputs");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* string_label = htab_find(htab_built_in, "string");
	htab_item_t* label_len = htab_find(htab_built_in, "len");
	htab_item_t* label_substr = htab_find(htab_built_in, "substr");

	generate_func_start(list, func);
	
	generate_instr(list, READ, 2, retval, string_label);
	
	htab_item_t* dlzka = generate_var(list, "dlzka", INT);

	generate_func_call(list, label_len, 1, retval);
	generate_save_return_value(list, dlzka);

	generate_instr(list, SUB, 3, dlzka, dlzka, make_const("one", INT, &one_1));

	generate_func_call(list, label_substr, 3, retval, make_const("zero", INT, &zero_0), dlzka);
	generate_save_return_value(list, retval);

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
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	
	generate_func_start(list, func);
	generate_instr(list, STRLEN, 2, retval, get_param(0));
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

	generate_instr(list, GT, 3, prava, get_param(1), make_const("minus_one" ,INT, &minus_one));
	generate_instr(list, LT, 3, lava, get_param(1), dlzka);

	/*htab_item_t* error_label = htab_find(htab_built_in, "error");
	generate_instr(list, JUMPIFNEQ, 3, error_label, prava, lava);*/

	generate_instr(list, STRI2INT, 3, retval, get_param(0), get_param(1));

	generate_func_end(list);
}

void generate_chr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "chr");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	
	generate_func_start(list, func);
	generate_instr(list, INT2CHAR, 2, retval, get_param(0));
	generate_func_end(list);
}

void generate_substr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "substr");
	htab_item_t* func_len = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	
	generate_func_start(list, func);

	generate_instr(list, MOVE, 2, retval, make_const("None", STRING, "")); // TODO None STRING

	htab_item_t* dlzka = generate_var(list, "dlzka", INT);
	generate_func_call(list, func_len, 1, get_param(0));

	generate_save_return_value(list, dlzka);	

	htab_item_t* podmienky = generate_var(list, "podmienky", BOOL);

	generate_instr(list, LT, 3, podmienky, dlzka, make_const("zero", INT, &zero_0));
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, make_const("const_true", BOOL, "true"));

	generate_instr(list, LT, 3, podmienky, get_param(1), make_const("zero", INT, &zero_0));
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, make_const("const_true", BOOL, "true"));

	generate_instr(list, EQ, 3, podmienky, get_param(1), dlzka);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, make_const("const_true", BOOL, "true"));

	generate_instr(list, GT, 3, podmienky, get_param(1), dlzka);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, make_const("const_true", BOOL, "true"));

	generate_instr(list, EQ, 3, podmienky, get_param(2), make_const("zero", INT, &zero_0));
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, make_const("const_true", BOOL, "true"));

	htab_item_t* ch = generate_var(list, "char", STRING);
	htab_item_t* idx = generate_var(list, "index", INT);
	generate_instr(list, MOVE, 2, idx, make_const("one", INT, &one_1));
	htab_item_t* last_idx = generate_var(list, "last_idx", INT);
	generate_instr(list, ADD, 3, last_idx, get_param(1), get_param(2));

	htab_item_t* loop_label = make_label("loop");
	htab_item_t* end_label = make_label("end");
	generate_instr(list, LABEL, 1, loop_label);
	generate_instr(list, GETCHAR, 3, ch, get_param(0), idx);
	generate_instr(list, CONCAT, 3, retval, retval, ch);
	generate_instr(list, ADD, 3, idx, idx, make_const("one", INT, &one_1));
	generate_instr(list, EQ, 3, podmienky, idx, last_idx);
	generate_instr(list, JUMPIFEQ, 3, end_label, podmienky, make_const("const_true", BOOL, "true"));
	generate_instr(list, LT, 3, podmienky, idx, dlzka);
	generate_instr(list, JUMPIFEQ, 3, loop_label, podmienky, make_const("const_true", BOOL, "true"));

	generate_instr(list, LABEL, 1, end_label);

	generate_func_end(list);
}

/*********************** INŠTRUKCIE ************************/

void generator_start(tList* list){
	htab_init(&htab_built_in);
	htab_init(&htab_tf);

	// premenné
	htab_insert(htab_built_in, "%retval", &zero_0, UNKNOWN, LF, false, false, true);
	// konštanta nil
	htab_insert(htab_built_in, "nil", "nil", NIL, LF, true, false, true);
	
	// funkcia main
	htab_insert(htab_built_in, "$main", "$main", FUNC, GF, false, true, true);

	// label error
	htab_insert(htab_built_in, "error", "error", FUNC, GF, false, true, true);

	// dátové tipy
	htab_insert(htab_built_in, "int", "int", TYPE_NAME, GF, false, true, true);
	htab_insert(htab_built_in, "float", "float", TYPE_NAME, GF, false, true, true);
	htab_insert(htab_built_in, "string", "string", TYPE_NAME, GF, false, true, true);

	htab_item_t* main_func = htab_find(htab_built_in, "$main");

	generate_first(list, JUMP, 1, main_func);	// skok na main
	generate_instr(list, LABEL, 1, main_func); 	// label na main
	main_func_node = list->last;

	// vstavané funkcie
	generate_len(list);	
	generate_ord(list);
	generate_chr(list);
	generate_substr(list);
	generate_inputs(list);
	generate_inputf(list);
	generate_inputi(list);
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
					switch(instr.param[i]->type){
						case INT: printf("int@%d", *instr.param[i]->value.ival); break;
						case FLOAT: printf("float@%lf", *instr.param[i]->value.dval); break;
						case STRING: printf("string@%s", instr.param[i]->value.sval); break;
						case BOOL: printf("bool@%s", instr.param[i]->value.sval); break;
						//default: printf("CHYBA_TYPE@"); break;
					};
				} else {
					//if(instr.param[i]->type != NIL){
					switch(instr.param[i]->frame){
						case GF: printf("GF@"); break;
						case LF: printf("LF@"); break;
						case TF: printf("TF@"); break;
						//default: printf("CHYBA_FRAME@"); break;
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

	DisposeList(list);
	htab_clear(htab_built_in);
	htab_clear(htab_tf);
}