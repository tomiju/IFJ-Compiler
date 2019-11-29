/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   generator.c
 * 
 *
 * Datum:    xx.xx.xxxx
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
tStack* else_nodes = NULL;
tStack* if_else_end_nodes = NULL;

htab_item_t* nil;

htab_item_t* params[20];

unsigned param_idx = 0;
unsigned par_count = 0;

int while_label_idx = -1;
int if_label_idx = -1;

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

tNode* tPopStack(tStack** stack){
	tStack* temp;

	temp = *stack;
	*stack = (*stack)->link;

	tNode* node = temp->node;
	free(temp);
	return node;
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

htab_item_t* get_param(unsigned idx){
	char arg_string[4] = "%";

	char idx_string[3];
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

htab_item_t* get_while_cond(){
	char cond_string[7] = "cond";

	char idx_string[3];
	sprintf(idx_string, "%d", while_label_idx);

	strcat(cond_string, idx_string);

	htab_item_t* item = htab_find(htab_built_in, cond_string);

	if(item == NULL){
		htab_insert(htab_built_in, cond_string, BOOL, GF, false, false, true);
		return htab_find(htab_built_in, cond_string);
	}

	return item;
	//return generate_var(list, cond_string, BOOL, GF);
}

htab_item_t* get_while_end(){
	char label_string[12] = "while_end";

	char idx_string[3];
	sprintf(idx_string, "%d", while_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_while_label(){
	char label_string[8] = "while";

	char idx_string[3];
	sprintf(idx_string, "%d", while_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

/*htab_item_t* get_if_cond(){
	char cond_string[10] = "cond_if";

	char idx_string[3];
	sprintf(idx_string, "%d", if_label_idx);

	strcat(cond_string, idx_string);

	return make_const(cond_string, BOOL);
}*/

htab_item_t* get_if_end(){
	char label_string[9] = "if_end";

	char idx_string[3];
	sprintf(idx_string, "%d", if_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_if_label(){
	char label_string[5] = "if";

	char idx_string[3];
	sprintf(idx_string, "%d", if_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_else_label(){
	char label_string[7] = "else";

	char idx_string[3];
	sprintf(idx_string, "%d", if_label_idx);

	strcat(label_string, idx_string);

	return make_label(label_string);
}

htab_item_t* get_if_bool(tList* list){
	char label_string[10] = "if_bool";

	char idx_string[3];
	sprintf(idx_string, "%d", if_label_idx);

	strcat(label_string, idx_string);

	return generate_var(list, label_string, BOOL, GF);
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
		//param = 0;
	}

	return param;
}

/*void generate_condition(tList* list){
	SetActive(list, if_condition);	
}*/

void start_if_else(tList* list){
	if_label_idx++;

	htab_item_t* true_const = make_const("true", BOOL);
	true_const->sval = "true";

	htab_item_t* if_bool = get_if_bool(list);
	tNode* if_condition = list->active;

	generate_instr(list, JUMPIFEQ, 3, get_if_label(), if_bool, true_const);
	generate_instr(list, JUMP, 1, get_else_label());

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
		htab_item_t* label_if = get_if_label();
		htab_item_t* label_else = get_else_label();

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

void generate_while_start(tList* list){
	while_label_idx++;

	generate_before_whiles(list, get_while_cond());

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
}

void send_param(htab_item_t* par){
	params[param_idx++] = par;
}

void func_call(tList* list, htab_item_t* func){
	htab_item_t* param;
	htab_item_t* val_to_copy;

	if(strcmp(func->key, "print") == 0){
		for(unsigned i = 0; i < param_idx; i++){
			generate_instr(list, CREATEFRAME, 0);

			val_to_copy = params[i];
			param = get_param_tf(0);

			generate_instr(list, DEFVAR, 1, param);
			generate_instr(list, MOVE, 2, param, val_to_copy);

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
	htab_item_t* dlzka = generate_var(list, "dlzka", INT, LF);

	generate_func_call(list, func_len, 1, get_param(0));
	generate_save_return_value(list, dlzka);

	htab_item_t* prava = generate_var(list, "prava", INT, LF);
	htab_item_t* lava = generate_var(list, "lava", INT, LF);

	htab_item_t* con = make_const("minus_one" ,INT);
	con->ival = -1;
	generate_instr(list, GT, 3, prava, get_param(1), con);
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

	htab_item_t* con = make_const("None", STRING);
	con->sval = "";
	generate_instr(list, MOVE, 2, retval, con); // TODO None STRING

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
	generate_instr(list, LT, 3, podmienky, dlzka, con_zero);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true );

	generate_instr(list, LT, 3, podmienky, get_param(1), con_zero);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	generate_instr(list, EQ, 3, podmienky, get_param(1), dlzka);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	generate_instr(list, GT, 3, podmienky, get_param(1), dlzka);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	generate_instr(list, EQ, 3, podmienky, get_param(2), con_zero);
	generate_instr(list, JUMPIFEQ, 3, make_label("end"), podmienky, con_true);

	htab_item_t* ch = generate_var(list, "char", STRING, LF);
	htab_item_t* idx = generate_var(list, "index", INT, LF);
	generate_instr(list, MOVE, 2, idx, con_one);
	htab_item_t* last_idx = generate_var(list, "last_idx", INT, LF);
	generate_instr(list, ADD, 3, last_idx, get_param(1), get_param(2));

	htab_item_t* loop_label = make_label("loop");
	htab_item_t* end_label = make_label("end");
	generate_instr(list, LABEL, 1, loop_label);
	generate_instr(list, GETCHAR, 3, ch, get_param(0), idx);
	generate_instr(list, CONCAT, 3, retval, retval, ch);
	generate_instr(list, ADD, 3, idx, idx, con_one);
	generate_instr(list, EQ, 3, podmienky, idx, last_idx);
	generate_instr(list, JUMPIFEQ, 3, end_label, podmienky, con_true);
	generate_instr(list, LT, 3, podmienky, idx, dlzka);
	generate_instr(list, JUMPIFEQ, 3, loop_label, podmienky, con_true);

	generate_instr(list, LABEL, 1, end_label);

	generate_func_end(list);
}

void generate_print(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "print");
	
	generate_func_start(list, func);
	
	htab_item_t* param = get_param(0);

	generate_instr(list, WRITE, 1, param);

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

	// label error
	//htab_insert(htab_built_in, "error", "error", FUNC, GF, false, true, true);

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
	generate_print(list);

	// pomocná premenná na kontrolu typov
	//generate_var(list, "%typ", BOOL, GF);
}

char* replace_by_escape(char* string){
	char* replaced = malloc(strlen(string)*3);

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

/*void check_types(tList* list, tInstr* instr){
	switch(instr->type){
		case ADD: case SUB: case MUL:	// oba int alebo float 
			break;
		case DIV:						// oba float
			break;	
		case IDIV:						// oba int
			break;	
		case CONCAT:					// oba string
			if(instr->param[0]->type == UNKNOWN){
				//generate_instr(list, TYPE, )
			}
			break;
	}	

	return;
}*/

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
						case INT: printf("int@%d", instr.param[i]->ival); break;
						case FLOAT: printf("float@%a", instr.param[i]->dval); break;
						case STRING: printf("string@%s", replace_by_escape(instr.param[i]->sval)); break;
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