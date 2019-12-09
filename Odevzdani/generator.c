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

#define ERR INTERNAL_ERROR

htab_t* htab_built_in; // symtable pre vstavané funkcie a pomocné premenné generátora
htab_t* htab_tf;	   // symtable pre dočasné parametre na local frame

tNode* main_func_node;	// ukazateľ na main v liste inštrukcií

tNode* before_while = NULL; // ukazateľ na inštrukciu pred while (vkladanie DEFVAR pred while)

// stacky pre vnorené if-else a while
tStack* while_nodes = NULL;
tStack* before_if_jumps = NULL;
tStack* if_nodes = NULL;
tStack* else_nodes = NULL;
tStack* if_else_end_nodes = NULL;

// stack pre indexi if-else
tStackNum* if_num_stack = NULL;

// predané argumenty do funkcie pred jej zavolaním
htab_item_t* params[256];
// počet parametrov
unsigned param_idx = 0;
unsigned par_count = 0;

// index posledného while
int while_label_idx = -1;

/************************ MAKRO ***********************/
// vytvorí pole stringov na výpis inštrukcií poďla indexu 
// ktorí je zadaný rovnomenným enumom
static const char* INSTR_STRING[] = {
    INSTR(GENERATE_STRING)
};

/********************* LIST INŠTRUKCIÍ *********************/

void InitList (tList* list){
	list->first = NULL;
	list->last = NULL;
	list->active = NULL;
}

void DisposeList(tList* list){
	tNode* act = list->first;
	tNode* del;

	while (act != NULL){
		del = act;
		act = del->next;

		// uvoľní parametre operandov
		for(int i = 0; i < 3; i++){
			free(del->instr.param[i]);
		}
		// uvoľní inštrukciu
		free(del);
	}

	list->last = NULL;
	list->first = NULL;
	list->active = NULL;
}

int InsertLast(tList* list, tInstr instr){
	tNode* new = malloc(sizeof(struct Node));

	if(new == NULL){
		return ERR;
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

void First(tList* list){
	list->active = list->first;
}

void Last(tList* list){
	list->active = list->last;
}

void SetActive(tList* list, tNode* node){
	list->active = node;
}

int PostInsert (tList* list, tInstr instr){
	if (list->active != NULL){
		tNode* new = malloc(sizeof(struct Node));

		if(new == NULL){
			return ERR;
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

tInstr* Copy (tList* list, tInstr* instr){
	if (list->active == NULL){
		return NULL;
	}

	*instr = list->active->instr;
	return 0;
}

void Succ (tList* list){
	if (list->active != NULL){
		list->active = list->active->next;
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

int InstrSetParam(tInstrPar** param, htab_item_t* htab_instr){
	(*param) = malloc(sizeof(struct InstrPar));

	if((*param) == NULL){
		return ERR;
	}

	// nastaví hodnotu podľa typu premennej
	switch(htab_instr->type){
		case INT: (*param)->ival = htab_instr->ival; break;
		case FLOAT: (*param)->dval = htab_instr->dval; break;
		case STRING: case BOOL: case NIL: (*param)->sval = htab_instr->sval; break;
	}

	// nastaví parametre pre operad inštrukcie
	(*param)->key = htab_instr->key;
	(*param)->type = htab_instr->type;
	(*param)->frame = htab_instr->frame;
	(*param)->isLabel = htab_instr->isLabel;
	(*param)->isConst = htab_instr->isConst;

	return 0;
}

/************************** STACK **************************/
int tPushStack(tStack** stack, tNode* node){
	tStack* temp = malloc(sizeof(struct stack));

	if(temp == NULL){
		return ERR;
	}

	temp->node = node;
	temp->link = *stack;

	*stack = temp;

	return 0;
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

int tPushStackNum(tStackNum** stack, int num){
	tStackNum* temp = malloc(sizeof(struct stackNum));

	if(temp == NULL){
		return ERR;
	}

	temp->num = num;
	temp->link = *stack;

	*stack = temp;

	return 0;
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

/******************** BLABLA *********************/

htab_item_t* generate_var(tList* list, char* name, int type, int frame){
	// vyhľadá premennú v tabuľke 
	htab_item_t* var = htab_find(htab_built_in, name);

	if(var == NULL){
		// ak ju nenašiel, vloží ju do tabuľky
		htab_insert(htab_built_in, name, type, frame, false, false, true);
		var = htab_find(htab_built_in, name);

		if(var == NULL){
			return NULL;
		}
	} else{
		var->frame = frame;
	}

	// nageneruje kód pre deklaráciu premmennje
	if(generate_instr_no(list, DEFVAR, 1, var) == ERR){
		return NULL;
	}
	return var;
}

htab_item_t* make_var(char* name, int type, int frame){
	// vyhľadá premennú v tabuľke 
	htab_item_t* var = htab_find(htab_built_in, name);

	if(var == NULL){
		// ak ju nenašiel, vloží ju do tabuľky
		htab_insert(htab_built_in, name, type, frame, false, false, true);
		var = htab_find(htab_built_in, name);
	} else{
		var->frame = frame;
	}

	return var;
}

htab_item_t* make_const(char* name, int type){
	// vyhľadá danú konštantu v tabuľke
	htab_item_t* var = htab_find(htab_built_in, name);
	if(var != NULL){
		var->type = type;

		return var;
	// ak sa nenašla, vloží ju do tabuľky
	} else{
		htab_insert(htab_built_in, name, type, LF, true, false, true);
		return htab_find(htab_built_in, name);
	}
}

// vytvorí label a vráti ukazateľ do tabuľky
htab_item_t* make_label(char* name){
	// vyhľadá label v tabuľke
	htab_item_t* label = htab_find(htab_built_in, name);
	
	if(label != NULL){
		return label;
	} else{
		// ak sa nenašiel, vloží ju do tabuľky
		htab_insert(htab_built_in, name, FUNC, LF, false, true, true);
		label = htab_find(htab_built_in, name);

		if(label == NULL){
			return NULL;
		}

		label->sval = name;
		return label;
	}
}

int generate_instr_no(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...){
	va_list list;

	tInstr instr;
	InstrInit(&instr, instr_enum);

	// prejde argumenty funkcie predané cez ...
	va_start(list, count);
	for(unsigned i = 0; i < count; i++){
		htab_item_t* htab_instr = va_arg(list, htab_item_t*);

		// nastaví parametre operandov inštrukcíi pre každý argument predyný do funkcie
		if(InstrSetParam(&(instr.param[i]), htab_instr) == ERR) return ERR;
	}
	va_end(list);

	// ak zoznam nie je prázdny, prvok sa vloží za aktívny a aktivita sa posunie o jeden prvok
	if(instr_list->first != NULL){
		if(PostInsert(instr_list, instr) == ERR) return ERR;
		Succ(instr_list);
	// ak je prázdny zoznam inštrukcií, nová inštrukcia sa vloží na začiatok, nastaví sa aktivita
	} else {
		if(InsertLast(instr_list, instr) == ERR) return ERR;
		First(instr_list);
	}

	return 0;
}

// generovanie kontrol dátových typov za behu
// vracia 0, ak sa negeneruje priamo inštrukciu so zmenenými dátovými typmi
// 1 pri generovaní inštrukcie a zmenenými operandmi
// 2 pri chybe
int check_types(tList* list, enum INSTR_ENUM instr_enum, htab_item_t** args){
	// určuje či došlo k konverzií typov
	bool converted = false;

	// vytvorenie konštant pre chybové návratové kódy
	htab_item_t* error_4 = make_const("err4", INT);
	htab_item_t* error_9 = make_const("err9", INT);

	if(error_4 == NULL || error_9 == NULL){
		return 2;
	}

	// nastavenie hodnôt pre konštanty
	error_4->ival = 4;
	error_9->ival = 9;

	// vytvorenie konštant typov pre porovnávanie
	htab_item_t* type_int = make_const("type_int", STRING); 
	htab_item_t* type_float = make_const("type_float", STRING);
	htab_item_t* type_nil = make_const("type_nil", STRING);
	htab_item_t* type_bool = make_const("type_bool", STRING);
	htab_item_t* type_string = make_const("type_string", STRING);

	if(type_int == NULL || type_float == NULL || type_nil == NULL || type_bool == NULL || type_string == NULL){
		return 2;
	}

	// nastavenie hodnôt
	type_int->sval = "int";
	type_float->sval = "float";
	type_nil->sval = "nil";
	type_bool->sval = "bool";
	type_string->sval = "string";

	// konštany typu INT a FLOAT s hodnotou 0 na porovnávanie
	htab_item_t* const_int_zero = make_const("const_int_zero", INT);
	htab_item_t* const_float_zero = make_const("const_float_zero", FLOAT);

	if(const_int_zero == NULL || const_float_zero == NULL){
		return 2;
	}

	// nastavenie hodnôt
	const_int_zero->ival = 0;
	const_float_zero->dval = 0.0;

	// pomocné premenné a ukladanie dátových typov premenných 
	htab_item_t* type1 = htab_find(htab_built_in, "%type1");
	htab_item_t* type2 = htab_find(htab_built_in, "%type2");

	if(type1 == NULL || type2 == NULL){
		return 2;
	}

	// pomocné premenné na typovú kontrolu a implicitné konverzie
	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_convert1 = htab_find(htab_built_in, "%type_converted1");
	htab_item_t* type_convert2 = htab_find(htab_built_in, "%type_converted2");

	if(type_control == NULL || type_convert1 == NULL || type_convert2 == NULL){
		return 2;
	}
 
	// konštanty true/false na náhradu za iné dátové typy ((0 || None || '') == FALSE, ináč TRUE ...)
	htab_item_t* con_true = make_const("const_true", BOOL);
	htab_item_t* con_false = make_const("const_false", BOOL);

	if(con_true == NULL || con_false == NULL){
		return 2;
	}

	con_true->sval = "true";
	con_false->sval = "false";

	// prázdny string ('')
	htab_item_t* empty_string = make_const("empty_string", STRING);

	if(empty_string == NULL){
		return 2;
	}

	empty_string->sval = "";

	// kontrola na základe konkrétnej inštrukcie
	switch(instr_enum){
		case LT: case GT: case EQ:
			// oba majú rovnaký typ (NIE UNKNOWN), preskočí,, nejde o konverziu
			if(args[1]->type != UNKNOWN && (args[1]->type == args[2]->type)){
				// ak ide o typ NIL a nejde o inštrukciu EQ, vygeneruje sa chybové ukončenie
				if(args[1]->type == NIL){
					if(instr_enum != EQ){		
						if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
					}
				}
				break;
			// aspoň jeden je NIL
			} else if(args[1]->type == NIL || args[2]->type == NIL){	
				// NIE EQ => nemôže byť NIL, ukončenie
				if(instr_enum != EQ){		
					if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
				}
			// ak je jeden INT a druhý float
			} else if(args[1]->type == INT && args[2]->type == FLOAT){
				// prevehne konverzia
				converted = true;
				if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]) == ERR) return ERR;
			// ak je prvý FLOAT a druhý INT
			} else if(args[1]->type == FLOAT && args[2]->type == INT){
				// prebehne konverzia
				converted = true;
				if(generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]) == ERR) return ERR;
				if(generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert2) == ERR) return ERR;
			// aspoň jeden operand je UNKNOWN
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){	
				converted = true;
			
				// skontroluje sa, či ide o rovnaké typy
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type1, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type2, args[2]) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, type1, type2) == ERR) return ERR;	
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// ak je inštrukcia EQ, môžme porovnávať akékoľvek rovnaké typy
					if(instr_enum == EQ){
						if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;
					// ináč skontrolujeme, či nejde o nil
					} else {
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type1, type_nil) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// vygeneruje sa chyba
							if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

						generate_else(list);

							// ináč sa vytvorí inštrukcia na porovnanie
							if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

						end_if_else(list);
					}

				// ak sú rozdielne zisťujeme typ
				generate_else(list);

					// ak je prvý typ int
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type1, type_int) == ERR) return ERR;
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;
					generate_if(list);

						// druhý typ je float
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type2, type_float) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// vykoná sa konverzia a vytvorí sa inštrukcia
							if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
							if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]) == ERR) return ERR;

						// ak nie je float
						generate_else(list);

							// ak ide o inštrukciu EQ, vieme porovnávať s NIL
							if(instr_enum == EQ){
								if(start_if_else(list) == ERR) return ERR;
								if(generate_instr_no(list, EQ, 3, type_control, type2, type_nil) == ERR) return ERR;
								if(generate_condition_check(list, type_control, false) == ERR) return ERR;
								generate_if(list);

									// EQ int == NIL
									if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

								generate_else(list);

									// chyba, zlé typy parametrov
									if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

								end_if_else(list);
							// ak nejde o inštrukciu EQ ide o chybu typov
							} else {
								if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
							}

						end_if_else(list);

					// ak prvý typ nie je INT
					generate_else(list);

						// skontrolujeme či ide o float
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type1, type_float) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// skontrolujeme, či je ďalší operand typu int
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, type2, type_int) == ERR) return ERR;
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								// ak áno, vykoná sa konverzia a inštrukcia
								if(generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]) == ERR) return ERR;
								if(generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert2) == ERR) return ERR;

							// ak nie je INT
							generate_else(list);

								// ak ide o inštrukciu EQ, skontrolujeme či je ďalší operad typu NIL
								if(instr_enum == EQ){
									if(start_if_else(list) == ERR) return ERR;
									if(generate_instr_no(list, EQ, 3, type_control, type2, type_nil) == ERR) return ERR;
									if(generate_condition_check(list, type_control, false) == ERR) return ERR;
									generate_if(list);

										// ak áno ide o validné porovanie
										if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

									generate_else(list);

										// ak nie, chyba
										if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

									end_if_else(list);
								// ak to nie je inštrukcia EQ, ide vždy o chybu
								} else {
									if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
								}

							end_if_else(list);

						// ak prvý operand nie je ani INT ani float
						generate_else(list);

							// ak ide o inštrukciu EQ, kontrolujeme či je nil
							if(instr_enum == EQ){
								if(start_if_else(list) == ERR) return ERR;
								if(generate_instr_no(list, EQ, 3, type_control, type1, type_nil) == ERR) return ERR;
								if(generate_condition_check(list, type_control, false) == ERR) return ERR;
								generate_if(list);

									// ak áno, validná kombinácia typov
									if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

								generate_else(list);

									// ináč môže byť druhý operand NIL
									if(start_if_else(list) == ERR) return ERR;
									if(generate_instr_no(list, EQ, 3, type_control, type2, type_nil) == ERR) return ERR;
									if(generate_condition_check(list, type_control, false) == ERR) return ERR;
									generate_if(list);

										// ak áno validná kombinácia
										if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

									generate_else(list);

										// ak nie chyba
										if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

									end_if_else(list);

								end_if_else(list);
							// ak nejde o EQ ide v každom prípade o chybu
							} else {
								if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
							}

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			// ináč ide o chybu
			} else {
				if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
			}
			break; 
		
		case ADD: case SUB: case MUL:
			// oba sú int, nedeje sa nič
			if(args[1]->type == INT && args[2]->type == INT){
				break;
			// oba sú float, nedeje sa nič
			} else if(args[1]->type == FLOAT && args[2]->type == FLOAT){
				break;
			// jeden je INT a druhý FLOAT, implicitná konverzia
			} else if(args[1]->type == INT && args[2]->type == FLOAT){
				converted = true;
				if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]) == ERR) return ERR;
			// jeden je FLOAT a druhý INT, implicitná konverzia
			} else if(args[1]->type == FLOAT && args[2] == INT){
				converted = true;
				if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]) == ERR) return ERR;
				if(generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1) == ERR) return ERR;
			// ak je aspoň jeden UNKNOWN
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){
				converted = true;
			
				// skontrujeme či ide o rovnaké typy
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type1, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type2, args[2]) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, type1, type2) == ERR) return ERR;	
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// ak sú oba INT, nageneruje sa inštrukcia 
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type1, type_int) == ERR) return ERR;	
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;
					generate_if(list);

						if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

					// ak nie, skontroujeme na FLOAT a nagenerujeme inštrukciu pri zhode
					generate_else(list);

						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type1, type_float) == ERR) return ERR;	
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

						// nie je ani INT ani FLOAT
						generate_else(list);

							// ak ide o inštrukciu ADD
							if(instr_enum == ADD){
								if(start_if_else(list) == ERR) return ERR;
								if(generate_instr_no(list, EQ, 3, type_control, type1, type_string) == ERR) return ERR;		
								if(generate_condition_check(list, type_control, false) == ERR) return ERR;
								generate_if(list);

									// a máme dva zhodné typy stringy, tak sa nageneruje CONCAT
									if(generate_instr_no(list, CONCAT, 3, args[0], args[1], args[2]) == ERR) return ERR;

								generate_else(list);

									// ináč chyba
									if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

								end_if_else(list);
							// pri iných inštrukciách chyba vždy
							} else {
								if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
							}
							
						end_if_else(list);

					end_if_else(list);

				// ak nemáme rovnaké typy
				generate_else(list);

					// ak je prvý INT
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type1, type_int) == ERR) return ERR;
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;
					generate_if(list);

						// a druhý je FLAOT
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type2, type_float) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// implicitná konverzia
							if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
							if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]) == ERR) return ERR;

						generate_else(list);

							// ináč chyba
							if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

						end_if_else(list);

					// ak nejde o INT
					generate_else(list);

						// ak je prvý FLOAT
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type1, type_float) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// druhý je INT
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, type2, type_int) == ERR) return ERR;
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								// implicitná konverzia
								if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]) == ERR) return ERR;
								if(generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1) == ERR) return ERR;

							generate_else(list);

								// ináč chyba 
								if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

							end_if_else(list);

						// ak nie je prvý typ ani INT ani FLOAT, chyba
						generate_else(list);

							if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			// iné kombinácie vedú na chybu
			} else {
				if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
			}
			break;
		
		case DIV: 
			converted = true;

			// oba FLOAT
			if(args[1]->type == FLOAT && args[2]->type == FLOAT){
				if(start_if_else(list) == ERR) return ERR;

				// skontroluje sa delenie 0
				if(generate_instr_no(list, EQ, 3, type_control, args[2], const_float_zero) == ERR) return ERR;	
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// delenie nulou vedie na chybu
					if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;	

				generate_else(list);

					// generovanie inštrukcie
					if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

				end_if_else(list);

			// jeden INT druhý FLOAT
			} else if(args[1]->type == INT && args[2]->type == FLOAT){
				// skontroluje sa delenie nulou
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[2], const_float_zero) == ERR) return ERR;	
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// delenie nulou vedie na chybu
					if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;	

				generate_else(list);

					// ináć sa prevedie INT na FLOAT a vytvorí sa inštrukcia
					if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
					if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]) == ERR) return ERR;

				end_if_else(list);
			// prvý FLOAT druhý je INT
			} else if(args[1]->type == FLOAT && args[2]->type == INT){
				// skontroluje sa delenie nulou
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// delenie nulou vedie na chybu
					if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;	

				generate_else(list);

					// INT sa prevedie na FLOAT a nageneruje sa inštrukcia
					if(generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]) == ERR) return ERR;
					if(generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert2) == ERR) return ERR;

				end_if_else(list);
			// oba sú INT
			} else if(args[1]->type == INT && args[2]->type == INT){
				// kontrola delenia nulou
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// pri delení 0 sa generuje chyba
					if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;	

				generate_else(list);

					// prevedú sa ona INT na FLAOT a prevedie sa inštrukcia
					if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
					if(generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]) == ERR) return ERR;
					if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, type_convert2) == ERR) return ERR;

				end_if_else(list);
			// aspoň jeden je unknown
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){	
				// skontroluje sa, či sú rovnaké typy		
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type1, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type2, args[2]) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, type1, type2) == ERR) return ERR;	
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// ak áno, skontrolujeme či sú oba INT
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type1, type_int) == ERR) return ERR;		
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;
					generate_if(list);

						// skontrolujeme delenie nulou
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero) == ERR) return ERR;		
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;	

						generate_else(list);

							if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
							if(generate_instr_no(list, INT2FLOAT, 2, type_convert2, args[2]) == ERR) return ERR;
							if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, type_convert2) == ERR) return ERR;

						end_if_else(list);

					// ak nie sú INT, skontrolujeme či nejde o FLOAT
					generate_else(list);

						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type1, type_float) == ERR) return ERR;	
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// kontrola delenia nulou
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, args[2], const_float_zero) == ERR) return ERR;	
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;
								
							generate_else(list);

								if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;
							
							end_if_else(list);

						// ináč chyba
						generate_else(list);

							if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

						end_if_else(list);

					end_if_else(list);
				// ak nie sú rovnaké dátové typy
				generate_else(list);

					// skontrolujeme či je prvý FLOAT
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type1, type_float) == ERR) return ERR;
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;
					generate_if(list);

						// ak je druhý INT
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type2, type_int) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// skontrolujeme delenie nulou
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero) == ERR) return ERR;	
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;	

							generate_else(list);

								if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[2]) == ERR) return ERR;
								if(generate_instr_no(list, instr_enum, 3, args[0], args[1], type_convert1) == ERR) return ERR;

							end_if_else(list);

						// ak je prvý FLOAT a druhý nie je INT, chyba
						generate_else(list);

							if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
							
						end_if_else(list);

					// skontrolujeme či je prvý INT
					generate_else(list);

						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type1, type_int) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// skontrolujeme druhý, či je FLOAT
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, type2, type_float) == ERR) return ERR;
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								// kontrola delenia nulou
								if(start_if_else(list) == ERR) return ERR;
								if(generate_instr_no(list, EQ, 3, type_control, args[2], const_float_zero) == ERR) return ERR;	
								if(generate_condition_check(list, type_control, false) == ERR) return ERR;
								generate_if(list);

									if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;		

								generate_else(list);

									if(generate_instr_no(list, INT2FLOAT, 2, type_convert1, args[1]) == ERR) return ERR;
									if(generate_instr_no(list, instr_enum, 3, args[0], type_convert1, args[2]) == ERR) return ERR;

								end_if_else(list);

							// ak je prvý FLOAT a druhý nie je INT -> chyba
							generate_else(list);

								if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;		

							end_if_else(list);

						// ak prvý je nie INT ani FLOAT -> chyba
						generate_else(list);

							if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			// ináč chyba
			} else {
				if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
			}
			break;
		case IDIV:
			converted = true;

			// ak sú oba INT
			if(args[1]->type == INT && args[2]->type == INT){
				// kontrola delenia nulou
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;

				generate_else(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

				end_if_else(list);
			// ak je aspoň jeden UNKNOWN
			} else if(args[1]->type == UNKNOWN || args[2]->type == UNKNOWN){
				// skontrolujeme či ide o rovnaké typy
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type1, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type2, args[2]) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, type1, type2) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// ak sú INT
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type1, type_int) == ERR) return ERR;		
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;
					generate_if(list);

						// kontrola delenia 0
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, args[2], const_int_zero) == ERR) return ERR;	
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							if(generate_instr_no(list, EXIT, 1, error_9) == ERR) return ERR;

						generate_else(list);

							if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

						end_if_else(list);

					// ak sú iné typy, chyba
					generate_else(list);

						if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

					end_if_else(list);

				// ak nie sú rovnaké typy chyba
				generate_else(list);

					if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

				end_if_else(list);
			// ináč chyba
			} else {
				if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
			}
			break;
		case JUMPIFEQ: case JUMPIFNEQ:
			converted = true;

			// ak je podmienka typu BOOL je to OK
			if(args[1]->type == BOOL){
				converted = false;

				break;
			// ak je INT
			} else if(args[1]->type == INT){
				// pre 0 generujeme FALSE, ináč 1
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[1], const_int_zero) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

				generate_else(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], con_true, args[2]) == ERR) return ERR;

				end_if_else(list);
			// ak je float
			} else if(args[1]->type == FLOAT){
				// pre 0 generujeme FALSE, ináč 1
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[1], const_float_zero) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

				generate_else(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], con_true, args[2]) == ERR) return ERR;

				end_if_else(list);
			// ak je podmienka STRING
			} else if(args[1]->type == STRING){
				// prázdny STRING '' -> FALSE ináč TRUE
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, args[1], empty_string) == ERR) return ERR;	
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

				generate_else(list);

					if(generate_instr_no(list, instr_enum, 3, args[0], con_true, args[2]) == ERR) return ERR;

				end_if_else(list);
			// ak NIL tak false
			} else if(args[1]->type == NIL){
				if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;
			// ak je UNKNOWN
			} else if(args[1]->type == UNKNOWN){
				// ak je bool
				if(start_if_else(list) == ERR) return ERR;
				if(generate_instr_no(list, TYPE, 2, type_control, args[1]) == ERR) return ERR;
				if(generate_instr_no(list, EQ, 3, type_control, type_control, type_bool) == ERR) return ERR;		
				if(generate_condition_check(list, type_control, false) == ERR) return ERR;
				generate_if(list);

					// generovanie inštrukcie
					if(generate_instr_no(list, instr_enum, 3, args[0], args[1], args[2]) == ERR) return ERR;

				// ak nie je bool
				generate_else(list);

					// skontrolujeme či ide o INT
					if(start_if_else(list) == ERR) return ERR;
					if(generate_instr_no(list, TYPE, 2, type_control, args[1]) == ERR) return ERR;
					if(generate_instr_no(list, EQ, 3, type_control, type_control, type_int) == ERR) return ERR;
					if(generate_condition_check(list, type_control, false) == ERR) return ERR;	
					generate_if(list);

						// ak 0 tak FALSE ináč TRUE
						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, args[1], const_int_zero) == ERR) return ERR;	
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

						generate_else(list);

							if(generate_instr_no(list, instr_enum, 3, args[0], con_true, args[2]) == ERR) return ERR;

						end_if_else(list);

					// ináč skontrolujeme či nejde o FLOAT
					generate_else(list);

						if(start_if_else(list) == ERR) return ERR;
						if(generate_instr_no(list, TYPE, 2, type_control, args[1]) == ERR) return ERR;
						if(generate_instr_no(list, EQ, 3, type_control, type_control, type_float) == ERR) return ERR;
						if(generate_condition_check(list, type_control, false) == ERR) return ERR;
						generate_if(list);

							// ak 0 tak FALSE ináč TRUE
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, args[1], const_float_zero) == ERR) return ERR;	
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

							generate_else(list);

								if(generate_instr_no(list, instr_enum, 3, args[0], con_true, args[2]) == ERR) return ERR;

							end_if_else(list);

						// skontrolujeme či nejde o string
						generate_else(list);

							// ak '' tak FALSE ináč TRUE
							if(start_if_else(list) == ERR) return ERR;
							if(generate_instr_no(list, TYPE, 2, type_control, args[1]) == ERR) return ERR;
							if(generate_instr_no(list, EQ, 3, type_control, type_control, type_string) == ERR) return ERR;		
							if(generate_condition_check(list, type_control, false) == ERR) return ERR;
							generate_if(list);

								if(start_if_else(list) == ERR) return ERR;
								if(generate_instr_no(list, EQ, 3, type_control, args[1], empty_string) == ERR) return ERR;		
								if(generate_condition_check(list, type_control, false) == ERR) return ERR;
								generate_if(list);

									if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

								generate_else(list);

									if(generate_instr_no(list, instr_enum, 3, args[0], con_true, args[2]) == ERR) return ERR;

								end_if_else(list);

							// ináč NIL -> FALSE
							generate_else(list);

								if(generate_instr_no(list, instr_enum, 3, args[0], con_false, args[2]) == ERR) return ERR;

							end_if_else(list);

						end_if_else(list);

					end_if_else(list);

				end_if_else(list);
			}
			break;
		default: break;
	}

	return converted;
}

int generate_instr(tList* instr_list, enum INSTR_ENUM instr_enum, unsigned count, ...){
	va_list list;
	htab_item_t** args = malloc(sizeof(htab_item_t*)*3);

	if(args == NULL){
		return ERR;
	}

	tInstr instr;
	InstrInit(&instr, instr_enum);

	// prejdeme zoznamom operandov inštrukcie
	va_start(list, count);
	for(unsigned i = 0; i < count; i++){
		htab_item_t* htab_instr = va_arg(list, htab_item_t*);
		args[i] = htab_instr;

		// nastaví parametre operandov inštrukcíi pre každý argument predyný do funkcie
		if(InstrSetParam(&(instr.param[i]), htab_instr) == ERR){
			return ERR;
		}
	}
	va_end(list);

	// skontrolujeme, či sú správne typy, pri chybe vraciame ERR
	int check = check_types(instr_list, instr_enum, args);
	if(check == 2){
		return ERR;
	// ak nedošlo k implicitnej konverzii
	} else if(check == 0){
		if(instr_list->first != NULL){
			if(PostInsert(instr_list, instr) == ERR) return ERR;
			Succ(instr_list);
		} else {
			if(InsertLast(instr_list, instr) == ERR) return ERR;
			First(instr_list);
		}
	}

	return 0;
}

// vytvorí názov premennej pre argumenty funkcie s indexom idx
htab_item_t* get_param(unsigned idx){
	char arg_string[5] = "%";

	// prevedie číslo indexu na string
	char idx_string[4];
	sprintf(idx_string, "%d", idx);

	// spojí index s názvom premennej
	strcat(arg_string, idx_string);

	// ak je v tabuľke, vráti ho, ak nie, vloží ho
	htab_item_t* param = htab_find(htab_built_in, arg_string);
	if(param == NULL){
		htab_insert(htab_built_in, arg_string, UNKNOWN, LF, false, false, true);
		param = htab_find(htab_built_in, arg_string);
	}

	return param;
}

// vytovrí string spojením name a idx ako indexu (napr "if3")
htab_item_t* get_string_with_index(char* name, int idx){
	// meno
	char label_string[20];
	strcpy(label_string, name);

	// identifikátor sa prevedie na string
	char idx_string[5];
	sprintf(idx_string, "%d", idx);

	// konkatenácia
	strcat(label_string, idx_string);

	// vytvorí sa label a vráti sa
	return make_label(label_string);
}

// vytvorí premennú na predávanie argumentvo do funkcie cez TF
htab_item_t* get_param_tf(unsigned idx){
	char arg_string[4] = "%";

	char idx_string[4];
	sprintf(idx_string, "%d", idx);

	strcat(arg_string, idx_string);

	// ak sa nenachádza v tabuľke, pridá ho do nej
	htab_item_t* param = htab_find(htab_tf, arg_string);
	if(param == NULL){
		htab_insert(htab_tf, arg_string, UNKNOWN, TF, false, false, true);
		param = htab_find(htab_tf, arg_string);
	}

	return param;
}

int start_if_else(tList* list){
	static int if_label_idx = -1;
	if_label_idx++;

	// pusche na stack aktuálny index IF-u s ktorým sa práve robí
	if(tPushStackNum(&if_num_stack, if_label_idx) == ERR) return ERR;

	// konštanta TRUE
	htab_item_t* true_const = make_const("true", BOOL);

	// label pre if
	htab_item_t* if_label = get_string_with_index("if", tTopStackNum(if_num_stack));
	htab_item_t* if_end = get_string_with_index("$if_end", tTopStackNum(if_num_stack));
	htab_item_t* else_label = get_string_with_index("else", tTopStackNum(if_num_stack));

	if(true_const == NULL || if_label == NULL || if_end == NULL || else_label == NULL){
		return ERR;
	}
	true_const->sval = "true";

	tNode* if_condition = list->active;

	// vytvorenie label pre if
	if(generate_instr(list, LABEL, 1, if_label) == ERR) return ERR;

	// pushne sa začiatok if pre neskorší skok
	if(tPushStack(&if_nodes, list->active) == ERR) return ERR;

	// nageneruje sa skok na end z IF a else label
	if(generate_instr(list, JUMP, 1, if_end) == ERR) return ERR;
	if(generate_instr(list, LABEL, 1, else_label) == ERR) return ERR;

	// pushne sa začiatok else pre neskorší skok
	if(tPushStack(&else_nodes, list->active) == ERR) return ERR;

	// label end
	if(generate_instr(list, LABEL, 1, if_end) == ERR) return ERR;

	// pushne sa na stack koniec if-elsu
	if(tPushStack(&if_else_end_nodes, list->active) == ERR) return ERR;

	// aktuálna inštrukcia sa nastaví na podmienku IF
	SetActive(list, if_condition);
	return 0;
}

int generate_condition_check(tList* list, htab_item_t* podmienka, bool isWhile){
	htab_item_t* con_true = make_const("const_true", BOOL);
	
	if(con_true == NULL){
		return ERR;
	}

	con_true->sval = "true";

	// ak ide o cyklus while
	if(isWhile){
		// skočí na koniec while ak nie je splnená podmienka
		htab_item_t* label_while_end = get_string_with_index("while_end", while_label_idx);

		if(label_while_end == NULL){
			return ERR;
		}

		if(generate_instr(list, JUMPIFNEQ, 3, label_while_end, podmienka, con_true) == ERR) return ERR;
	} else {
		htab_item_t* label_else = get_string_with_index("else", tTopStackNum(if_num_stack));
		htab_item_t* label_if = get_string_with_index("if", tTopStackNum(if_num_stack));
		
		if(label_else == NULL || label_if == NULL){
			return ERR;
		}

		// pushne sa na stack pozicía začiatku podmienky (ak treba nagenerovať DEFVAR pred if)
		if(tPushStack(&before_if_jumps, list->active) == ERR) return ERR;

		// nageneruje sa skok na if, ak nie je splnená podmienka, skáče na else
		if(generate_instr(list, JUMPIFEQ, 3, label_if, podmienka, con_true) == ERR) return ERR;
		if(generate_instr(list, JUMP, 1, label_else) == ERR) return ERR;
	}

	return 0;
}

void generate_if(tList* list){
	// nastaví aktivitu na začiatok ifu
	SetActive(list, tPopStack(&if_nodes));
}

void generate_else(tList* list){
	// nastaví aktivitu na začiatok elsu
	SetActive(list, tPopStack(&else_nodes));
}

void end_if_else(tList* list){
	// nastaví aktivitu na koniec if-elsu, zároveň popne potrebné stacky
	SetActive(list, tPopStack(&if_else_end_nodes));
	tPopStackNum(&if_num_stack);
	tPopStack(&before_if_jumps);
}

int generate_before_whiles(tList* list, htab_item_t* item){
	tNode* temp = list->active;

	// ak je nejaký while, nastaví aktivitu pred jeho začiatok
	if(before_while != NULL){
		SetActive(list, before_while);
	}

	// nageneruje definíciu premennej
	if(generate_instr(list, DEFVAR, 1, item) == ERR) return ERR;

	// vráti aktivitu do pôvodného stavu
	if(before_while != NULL){
		SetActive(list, temp);
		before_while = before_while->next;
	}

	return 0;
}

int generate_before_if(tList* list, htab_item_t* item){
	tNode* temp = list->active;
	tNode* before_jump = tTopStack(before_if_jumps);

	// ak je nejaké if, nastaví aktivitu, nageneruje definíciu premennej pred if a vráti aktivitu listu
	if(before_jump != NULL){
		SetActive(list, before_jump);
		if(generate_instr(list, DEFVAR, 1, item) == ERR) return ERR;
		SetActive(list, temp);
	} else {
		if(generate_instr(list, DEFVAR, 1, item) == ERR) return ERR;
	}

	return 0;
}

int generate_while_start(tList* list){
	while_label_idx++;

	if(before_while == NULL){
		before_while = list->active;
	}

	htab_item_t* while_label = get_string_with_index("while", while_label_idx);
	htab_item_t* while_end_label = get_string_with_index("while_end", while_label_idx);

	if(while_label == NULL || while_end_label == NULL){
		return ERR;
	}

	// label pre while
	if(generate_instr(list, LABEL, 1, while_label) == ERR) return ERR;

	tNode* node = list->active;

	// skok na while label a label koniec while
	if(generate_instr(list, JUMP, 1, while_label) == ERR) return ERR;
	if(generate_instr(list, LABEL, 1, while_end_label) == ERR) return ERR;

	// pushne na stack začiatok tela while
	if(tPushStack(&while_nodes, list->active) == ERR) return ERR;

	SetActive(list, node);

	return 0;
}

void generate_while_end(tList* list){
	SetActive(list, tPopStack(&while_nodes));

	if(while_nodes == NULL){
		before_while = NULL;
	}
}

// vytovrí pre funkciu návratovú hodnotu a nastaví ju na NIL
int generate_return_variable(tList* list_instr){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");

	// konštanta nil
	htab_insert(htab_built_in, "nil", NIL, LF, true, false, true);
	htab_item_t* nil = htab_find(htab_built_in, "nil");

	if(nil == NULL || ret_val == NULL){
		return ERR;
	}

	ret_val->frame = LF;

	nil->sval = "nil";

	if(generate_instr(list_instr, DEFVAR, 1, ret_val) == ERR) return ERR;
	if(generate_instr(list_instr, MOVE, 2, ret_val, nil) == ERR) return ERR;

	return 0;
}

int generate_func_start(tList* list, htab_item_t* label){
	SetActive(list, main_func_node->prev);

	// vytovrí label a rámec pre funkciu
	if(generate_instr(list, LABEL, 1, label) == ERR) return ERR;
	if(generate_instr(list, PUSHFRAME, 0) == ERR) return ERR;

	// vytovrí návratovú hodnotu a nastaví na NIL
	if(generate_return_variable(list) == ERR) return ERR;

	return 0;
}

int generate_save_to_return(tList* list, htab_item_t* value_to_save){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");

	if(ret_val == NULL){
		return ERR;
	}

	// nastaví návratovú hodnotu ako hodnotu z premennej value_to_save
	if(generate_instr(list, MOVE, 2, ret_val, value_to_save) == ERR) return ERR;

	return 0;
}

int generate_return(tList* list){
	// nastaví koniec funkcie
	if(generate_instr(list, POPFRAME, 0) == ERR) return ERR;
	if(generate_instr(list, RETURN, 0) == ERR) return ERR;

	return 0;
}

int generate_func_end(tList* list){
	// nastaví koniec funkcie
	if(generate_instr(list, POPFRAME, 0) == ERR) return ERR;
	if(generate_instr(list, RETURN, 0) == ERR) return ERR;

	// nastaví aktivitu v liste na koniec (funkcie sa generujú pred main)
	Last(list);

	return 0;
}

int generate_save_return_value(tList* list, htab_item_t* var){
	htab_item_t* ret_val = htab_find(htab_built_in, "%retval");

	if(ret_val == NULL){
		return ERR;
	}

	ret_val->frame = TF;

	if(generate_instr(list, MOVE, 2, var, ret_val) == ERR) return ERR;
	ret_val->frame = LF;

	return 0;
}

void send_param(htab_item_t* par){
	// pridá ďalší parameter pre funkciu do poľa 
	params[param_idx++] = par;
}

int func_call(tList* list, htab_item_t* func){
	htab_item_t* param;
	htab_item_t* val_to_copy;

	// pomocné premenné
	htab_item_t* space_const = make_const("medzera", STRING);
	htab_item_t* new_line_const = make_const("odriadkovanies", STRING);
	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_nil = make_const("type_nil", STRING);
	htab_item_t* string_none = make_const("string_none", STRING);
	
	if(space_const == NULL || new_line_const == NULL || type_control == NULL || type_nil == NULL || string_none == NULL){
		return ERR;
	}

	space_const->sval = " ";
	new_line_const->sval = "\n";
	type_nil->sval = "nil";
	string_none->sval = "None";

	htab_item_t* param0 = get_param_tf(0);

	if(param0 == NULL){
		return ERR;
	}

	// ak ide o funkciu print
	if(strcmp(func->key, "print") == 0){
		// ak nemá žiaden argument, generuje sa '\n' ako výpis
		if(param_idx == 0){
			if(generate_instr(list, CREATEFRAME, 0) == ERR) return ERR;
			if(generate_instr(list, DEFVAR, 1, param0) == ERR) return ERR;
			if(generate_instr(list, MOVE, 2, param0, new_line_const) == ERR) return ERR;
			if(generate_instr(list, CALL, 1, func) == ERR) return ERR;
		}

		// pre každý argument sa nageneruje volanie printu
		for(unsigned i = 0; i < param_idx; i++){
			val_to_copy = params[i];
			param = get_param_tf(0);

			if(param == NULL){
				return ERR;
			}

			// ak ide o NIL, nageneruje sa string None
			if(start_if_else(list) == ERR) return ERR;
			if(generate_instr_no(list, TYPE, 2, type_control, val_to_copy) == ERR) return ERR;
			if(generate_instr_no(list, EQ, 3, type_control, type_control, type_nil) == ERR) return ERR;		
			if(generate_condition_check(list, type_control, false) == ERR) return ERR;
			generate_if(list);

				if(generate_instr(list, CREATEFRAME, 0) == ERR) return ERR;
				if(generate_instr(list, DEFVAR, 1, param) == ERR) return ERR;
				if(generate_instr(list, MOVE, 2, param, string_none) == ERR) return ERR;
				if(generate_instr(list, CALL, 1, func) == ERR) return ERR;

			// ináč sa predá argument
			generate_else(list);

				if(generate_instr(list, CREATEFRAME, 0) == ERR) return ERR;
				if(generate_instr(list, DEFVAR, 1, param) == ERR) return ERR;
				if(generate_instr(list, MOVE, 2, param, val_to_copy) == ERR) return ERR;
				if(generate_instr(list, CALL, 1, func) == ERR) return ERR;

			end_if_else(list);

			// vytvorí sa rámec na predanie argumentu
			if(generate_instr(list, CREATEFRAME, 0) == ERR) return ERR;

			// ak ide o posledný argument, nageneruje sa '\n', ináč ' '
			if(param_idx != (i + 1)){
				if(generate_instr(list, DEFVAR, 1, param) == ERR) return ERR;
				if(generate_instr(list, MOVE, 2, param, space_const) == ERR) return ERR;
			}else {
				if(generate_instr(list, DEFVAR, 1, param) == ERR) return ERR;
				if(generate_instr(list, MOVE, 2, param, new_line_const) == ERR) return ERR;
			}

			// zavolá sa print
			if(generate_instr(list, CALL, 1, func) == ERR) return ERR;
		}

		par_count = 0;
		param_idx = 0;
	// ak nejde o print
	} else {
		// vytvorí sa rámec pre funkciu
		if(generate_instr(list, CREATEFRAME, 0) == ERR) return ERR;

		// pre každý argument sa vytovŕi pomocná premenná na predanie cez TF
		for(unsigned i = 0; i < param_idx; i++){
			val_to_copy = params[i];
			param = get_param_tf(i);

			if(generate_instr(list, DEFVAR, 1, param) == ERR) return ERR;
			if(generate_instr(list, MOVE, 2, param, val_to_copy) == ERR) return ERR;
		}

		// zavolá sa funkcia
		if(generate_instr(list, CALL, 1, func) == ERR) return ERR;

		par_count = param_idx;
		param_idx = 0;
	}

	return 0;
}

// volanie func_call ale s predaním všetkých argumentov naraz cez ...
int generate_func_call(tList* list, htab_item_t* label, unsigned count, ...){
	va_list arg_list;

	htab_item_t* param;
	htab_item_t* val_to_copy;

	// vytovrí sa rámec pre argumenty
	if(generate_instr(list, CREATEFRAME, 0) == ERR) return ERR;

	va_start(arg_list, count);

	// prejdú sa všetky argumenty
	for(unsigned i = 0; i < count; i++){
		val_to_copy = va_arg(arg_list, htab_item_t*);
		param = get_param_tf(i);

		if(param == NULL){
			return ERR;
		}

		if(generate_instr(list, DEFVAR, 1, param) == ERR) return ERR;
		if(generate_instr(list, MOVE, 2, param, val_to_copy) == ERR) return ERR;
	}

	va_end(arg_list);

	if(generate_instr(list, CALL, 1, label) == ERR) return ERR;

	return 0;
}

/******************** VSTAVANÉ FUNKCIE *********************/

int generate_inputs(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputs");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* string_label = htab_find(htab_built_in, "string");

	if(func == NULL || retval == NULL || string_label == NULL){
		return ERR;
	}

	// nageneruje začiatok funkcie, prečítanie hodnoty STRING a koniec
	if(generate_func_start(list, func) == ERR) return ERR;
	if(generate_instr_no(list, READ, 2, retval, string_label) == ERR) return ERR;
	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_inputf(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputf");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* float_label = htab_find(htab_built_in, "float");

	if(func == NULL || retval == NULL || float_label == NULL){
		return ERR;
	}

	// nageneruje začiatok funkcie, prečítanie hodnoty FLOAT a koniec
	if(generate_func_start(list, func) == ERR) return ERR;
	if(generate_instr_no(list, READ, 2, retval, float_label) == ERR) return ERR;
	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_inputi(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "inputi");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");
	htab_item_t* int_label = htab_find(htab_built_in, "int");

	if(func == NULL || retval == NULL || int_label == NULL){
		return ERR;
	}

	// nageneruje začiatok funkcie, prečítanie hodnoty INT a koniec
	if(generate_func_start(list, func) == ERR) return ERR;
	if(generate_instr_no(list, READ, 2, retval, int_label) == ERR) return ERR;
	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_len(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_string = make_const("type_string", STRING);

	htab_item_t* error_4 = make_const("err4", INT);

	if(func == NULL || retval == NULL || type_control == NULL || type_string == NULL || error_4 == NULL){
		return ERR;
	}

	type_string->sval = "string";

	// nageneruje začiatok funkcie
	if(generate_func_start(list, func) == ERR) return ERR;

	// skontroluje či ide o string a nageneruje inštrukciu na výpočet dĺžky, ináč chyba
	if(start_if_else(list) == ERR) return ERR;
	if(generate_instr_no(list, TYPE, 2, type_control, get_param(0)) == ERR) return ERR;
	if(generate_instr_no(list, EQ, 3, type_control, type_control, type_string) == ERR) return ERR;
	if(generate_condition_check(list, type_control, false) == ERR) return ERR;
	generate_if(list);

		if(generate_instr_no(list, STRLEN, 2, retval, get_param(0)) == ERR) return ERR;

	generate_else(list);

		if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
	
	end_if_else(list);
	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_ord(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "ord");
	htab_item_t* func_len = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_string = make_const("type_string", STRING);
	htab_item_t* type_int = make_const("type_int", STRING); 

	htab_item_t* error_4 = make_const("err4", INT);

	htab_item_t* param0 =  get_param(0);
	htab_item_t* param1 =  get_param(1);

	if(func == NULL || func_len == NULL || retval == NULL || type_control == NULL || type_string == NULL || type_int == NULL || error_4 == NULL || param0 == NULL || param1 == NULL){
		return ERR;
	}

	type_string->sval = "string";
	type_int->sval = "int";

	// začiatok funckie
	if(generate_func_start(list, func) == ERR) return ERR;

	/// ak je 0 argument typu STRING
	if(start_if_else(list) == ERR) return ERR;
	if(generate_instr_no(list, TYPE, 2, type_control, param0) == ERR) return ERR;
	if(generate_instr_no(list, EQ, 3, type_control, type_control, type_string) == ERR) return ERR;
	if(generate_condition_check(list, type_control, false) == ERR) return ERR;
	generate_if(list);

		// ak je prvý argumenty typu INT, všetko je OK pass
		if(start_if_else(list) == ERR) return ERR;
		if(generate_instr_no(list, TYPE, 2, type_control, param1) == ERR) return ERR;
		if(generate_instr_no(list, EQ, 3, type_control, type_control, type_int) == ERR) return ERR;
		if(generate_condition_check(list, type_control, false) == ERR) return ERR;
		generate_if(list);

			// pass

		// ak nie je, chyba
		generate_else(list);

			if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
		
		end_if_else(list);

	// ak nie je prvý argument string, chyba
	generate_else(list);

		if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
	
	end_if_else(list);

	htab_item_t* dlzka = generate_var(list, "dlzka", INT, LF);

	if(dlzka == NULL){
		return ERR;
	}

	// zavolá sa funkcia len a uloží sa výsledok
	if(generate_func_call(list, func_len, 1, get_param(0)) == ERR) return ERR;
	if(generate_save_return_value(list, dlzka) == ERR) return ERR;

	htab_item_t* prava = generate_var(list, "prava", INT, LF);
	htab_item_t* lava = generate_var(list, "lava", INT, LF);

	htab_item_t* con = make_const("minus_one", INT);

	if(prava == NULL || lava == NULL || con == NULL){
		return ERR;
	}

	con->ival = -1;

	// otesutujú sa hranice pre prvý parameter (indexované od 0)
	if(generate_instr_no(list, GT, 3, prava, get_param(1), con) == ERR) return ERR;
	if(generate_instr_no(list, LT, 3, lava, get_param(1), dlzka) == ERR) return ERR;

	htab_item_t* error_label = make_label("error");

	if(error_label == NULL){
		return ERR;
	}

	error_label->sval = "error_label";

	// ak je mimo hraníc, skočí sa na error (nenastavuje sa návratová hodnota -> ostáva na NIL)
	if(generate_instr_no(list, JUMPIFNEQ, 3, error_label, prava, lava) == ERR) return ERR;
	if(generate_instr_no(list, STRI2INT, 3, retval, get_param(0), get_param(1)) == ERR) return ERR;
	if(generate_instr_no(list, LABEL, 1, error_label) == ERR) return ERR;

	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_chr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "chr");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_int = make_const("type_int", STRING); 

	htab_item_t* error_4 = make_const("err4", INT);
	htab_item_t* zero = make_const("zero", INT);
	htab_item_t* int_255 = make_const("int_255", INT);

	htab_item_t* param0 = get_param(0);

	if(func == NULL || retval == NULL || type_control == NULL || type_int == NULL || error_4 == NULL || zero == NULL || int_255 == NULL || param0 == NULL){
		return ERR;
	}
	
	type_int->sval = "int";

	zero->ival = 0;
	int_255->ival = 255;

	if(generate_func_start(list, func) == ERR) return ERR;

	htab_item_t* prava = generate_var(list, "prava", INT, LF);
	htab_item_t* lava = generate_var(list, "lava", INT, LF);

	if(lava == NULL || prava == NULL){
		return ERR;
	}

	// ak je 0. argument INT
	if(start_if_else(list) == ERR) return ERR;
	if(generate_instr_no(list, TYPE, 2, type_control, param0) == ERR) return ERR;
	if(generate_instr_no(list, EQ, 3, type_control, type_control, type_int) == ERR) return ERR;
	if(generate_condition_check(list, type_control, false) == ERR) return ERR;
	generate_if(list);

		// ak je mimo hraníc, chyba ináč ok
		if(start_if_else(list) == ERR) return ERR;
		if(generate_instr_no(list, GT, 3, prava, param0, int_255) == ERR) return ERR;
		if(generate_instr_no(list, LT, 3, lava, param0, zero) == ERR) return ERR;
		if(generate_instr_no(list, EQ, 3, type_control, lava, prava) == ERR) return ERR;
		if(generate_condition_check(list, type_control, false) == ERR) return ERR;

		generate_if(list);

			if(generate_instr_no(list, INT2CHAR, 2, retval, param0) == ERR) return ERR;

		generate_else(list);

			if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;

		end_if_else(list);

	// ak nie je INT, chyba
	generate_else(list);

		if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
	
	end_if_else(list);

	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_substr(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "substr");
	htab_item_t* func_len = htab_find(htab_built_in, "len");
	htab_item_t* retval = htab_find(htab_built_in, "%retval");

	htab_item_t* type_control = htab_find(htab_built_in, "%type_control");
	htab_item_t* type_int = make_const("type_int", STRING); 
	htab_item_t* type_string = make_const("type_string", STRING);

	htab_item_t* error_4 = make_const("err4", INT);	

	htab_item_t* param0 = get_param(0);
	htab_item_t* param1 = get_param(1);
	htab_item_t* param2 = get_param(2);
	htab_item_t* label_end = make_label("end");

	if(func == NULL || func_len == NULL || retval == NULL || type_control == NULL || type_int == NULL || type_string == NULL
	|| error_4 == NULL || param0 == NULL || param1 == NULL || param2 == NULL || label_end == NULL){
		return ERR;
	}

	type_int->sval = "int";
	type_string->sval = "string";

	if(generate_func_start(list, func) == ERR) return ERR;

	// skontrolu je či je 0. (od 0) argument STRING
	if(start_if_else(list) == ERR) return ERR;
	if(generate_instr_no(list, TYPE, 2, type_control, param0) == ERR) return ERR;
	if(generate_instr_no(list, EQ, 3, type_control, type_control, type_string) == ERR) return ERR;
	if(generate_condition_check(list, type_control, false) == ERR) return ERR;
	generate_if(list);

		// skontroluje či je 1. argument INT
		if(start_if_else(list) == ERR) return ERR;
		if(generate_instr_no(list, TYPE, 2, type_control, param1) == ERR) return ERR;
		if(generate_instr_no(list, EQ, 3, type_control, type_control, type_int) == ERR) return ERR;
		if(generate_condition_check(list, type_control, false) == ERR) return ERR;
		generate_if(list);

			// skontroluje či je 2. argumetn INT
			if(start_if_else(list) == ERR) return ERR;
			if(generate_instr_no(list, TYPE, 2, type_control, param2) == ERR) return ERR;
			if(generate_instr_no(list, EQ, 3, type_control, type_control, type_int) == ERR) return ERR;
			if(generate_condition_check(list, type_control, false) == ERR) return ERR;
			generate_if(list);

				// ak áno OK, pass

			generate_else(list);

				if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
			
			end_if_else(list);

		generate_else(list);

			if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
		
		end_if_else(list);

	generate_else(list);

		if(generate_instr_no(list, EXIT, 1, error_4) == ERR) return ERR;
	
	end_if_else(list);

	htab_item_t* con = generate_var(list, "%tmp_str", STRING, LF);
	htab_item_t* empty_string = make_const("empty_string", STRING);

	if(con == NULL || empty_string == NULL){
		return ERR;
	}
	
	con->sval = "";
	empty_string->sval = "";

	// do con sa bude pridávať znak po znaku, na začiatok prázdny string
	if(generate_instr_no(list, MOVE, 2, con, empty_string) == ERR) return ERR;

	htab_item_t* dlzka = generate_var(list, "dlzka", INT, LF);

	if(dlzka == NULL){
		return ERR;
	}

	// zavolá sa funkcia dĺžky nad 0 argumentom
	if(generate_func_call(list, func_len, 1, param0) == ERR) return ERR;

	if(generate_save_return_value(list, dlzka) == ERR) return ERR;

	htab_item_t* podmienky = generate_var(list, "podmienky", BOOL, LF);

	htab_item_t* con_zero = make_const("zero", INT);
	htab_item_t* con_one = make_const("one", INT);

	htab_item_t* con_true = make_const("const_true", BOOL);

	if(podmienky == NULL || con_zero == NULL || con_one == NULL || con_true == NULL){
		return ERR;
	}
	
	con_zero->ival = 0;
	con_one->ival = 1;
	con_true->sval = "true";

	// ak je dĺžka 0, skočí sa na koniec
	if(generate_instr_no(list, LT, 3, podmienky, dlzka, con_zero) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, label_end, podmienky, con_true) == ERR) return ERR;

	// ak je 1. argument 0 menší ako, skočí sa na koniec
	if(generate_instr_no(list, LT, 3, podmienky, param1, con_zero) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, label_end, podmienky, con_true) == ERR) return ERR;

	// ak je 1. argument 0  rovný dĺžke reťazca, skočí sa na koniec
	if(generate_instr_no(list, EQ, 3, podmienky, param1, dlzka) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, label_end, podmienky, con_true) == ERR) return ERR;

	// ak je 1. argument 0  vačší ako dĺžka reťazca, skočí sa na koniec
	if(generate_instr_no(list, GT, 3, podmienky, param1, dlzka) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, label_end, podmienky, con_true) == ERR) return ERR;

	// ak je 2. argument == 0, skočí sa na koniec
	if(generate_instr_no(list, EQ, 3, podmienky, param2, con_zero) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, label_end, podmienky, con_true) == ERR) return ERR;

	htab_item_t* ch = generate_var(list, "char", STRING, LF);
	htab_item_t* idx = generate_var(list, "index", INT, LF);
	htab_item_t* last_idx = generate_var(list, "last_idx", INT, LF);

	if(ch == NULL || idx == NULL || last_idx == NULL){
		return ERR;
	}

	// do idx načítame prvý idex z 1. parametru a vypočítame posledný idx
	if(generate_instr_no(list, MOVE, 2, idx, param1) == ERR) return ERR;
	if(generate_instr_no(list, ADD, 3, last_idx, param1, param2) == ERR) return ERR;

	htab_item_t* loop_label = make_label("loop");
	htab_item_t* end_label = make_label("end");

	if(loop_label == NULL || end_label == NULL){
		return ERR;
	}

	// cyklus, pridávamé chary do stringu až kým nie sme mimo stringu, alebo sme narazili na last_idx
	if(generate_instr_no(list, LABEL, 1, loop_label) == ERR) return ERR;
	if(generate_instr_no(list, GETCHAR, 3, ch, param0, idx) == ERR) return ERR;
	if(generate_instr_no(list, CONCAT, 3, con, con, ch) == ERR) return ERR;
	if(generate_instr_no(list, ADD, 3, idx, idx, con_one) == ERR) return ERR;
	if(generate_instr_no(list, MOVE, 2, retval, con) == ERR) return ERR;
	if(generate_instr_no(list, EQ, 3, podmienky, idx, last_idx) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, end_label, podmienky, con_true) == ERR) return ERR;
	if(generate_instr_no(list, LT, 3, podmienky, idx, dlzka) == ERR) return ERR;
	if(generate_instr_no(list, JUMPIFEQ, 3, loop_label, podmienky, con_true) == ERR) return ERR;

	if(generate_instr_no(list, LABEL, 1, end_label) == ERR) return ERR;

	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generate_print(tList* list){
	htab_item_t* func = htab_find(htab_built_in, "print");
	htab_item_t* param = get_param(0);

	if(func == NULL || param == NULL){
		return ERR;
	}

	// vypíše jeden argument 
	if(generate_func_start(list, func) == ERR) return ERR;
	if(generate_instr_no(list, WRITE, 1, param) == ERR) return ERR;
	if(generate_func_end(list) == ERR) return ERR;

	return 0;
}

int generator_start(tList* list){
	if(htab_init(&htab_built_in) == ERR) return ERR;
	if(htab_init(&htab_tf) == ERR) return ERR;

	// premenné
	if(htab_insert(htab_built_in, "%retval", UNKNOWN, LF, false, false, true) == ERR) return ERR;
	
	// funkcia main
	if(htab_insert(htab_built_in, "$main", FUNC, GF, false, true, true) == ERR) return ERR;
	htab_item_t* main_func = htab_find(htab_built_in, "$main");

	if(main_func == NULL){
		return ERR;
	}

	main_func->sval = "$main";

	// dátové tipy
	if(htab_insert(htab_built_in, "int", TYPE_NAME, GF, false, true, true) == ERR) return ERR;
	if(htab_insert(htab_built_in, "float", TYPE_NAME, GF, false, true, true) == ERR) return ERR;
	if(htab_insert(htab_built_in, "string", TYPE_NAME, GF, false, true, true) == ERR) return ERR;
	
	htab_item_t* item_int = htab_find(htab_built_in, "int");
	htab_item_t* item_float = htab_find(htab_built_in, "float");
	htab_item_t* item_string = htab_find(htab_built_in, "string");
		
	if(item_int == NULL || item_float == NULL || item_string == NULL){
		return ERR;
	}

	item_int->sval = "int";
	item_float->sval = "float";
	item_string->sval = "string";

	// skok a label na main
	if(generate_instr_no(list, JUMP, 1, main_func) == ERR) return ERR;	
	if(generate_instr_no(list, LABEL, 1, main_func) == ERR) return ERR;
	main_func_node = list->last;

	// pomocné premenné na kontrolu typov
	if(generate_var(list, "%type_control", BOOL, GF) == NULL) return ERR;
	if(generate_var(list, "%type_converted1", FLOAT, GF) == NULL) return ERR;
	if(generate_var(list, "%type_converted2", FLOAT, GF) == NULL) return ERR;
	if(generate_var(list, "%type1", UNKNOWN, GF) == NULL) return ERR;
	if(generate_var(list, "%type2", UNKNOWN, GF) == NULL) return ERR;

	// vstavané funkcie
	if(generate_len(list) == ERR) return ERR;
	if(generate_ord(list) == ERR) return ERR;
	if(generate_chr(list) == ERR) return ERR;
	if(generate_substr(list) == ERR) return ERR;
	if(generate_inputs(list) == ERR) return ERR;
	if(generate_inputf(list) == ERR) return ERR;
	if(generate_inputi(list) == ERR) return ERR;
	if(generate_print(list) == ERR) return ERR;

	return 0;
}

// nahradí v stringu všetky escape sekvencie za \\xxx hodnotu
char* replace_by_escape(char* string){
	char* replaced = malloc((strlen(string)+1)*4);

	if(replaced == NULL){
		return NULL;
	}

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

int printInstructions(tList* list){
	printf(".IFJcode19\n");

	First(list);
	tInstr instr;

	while(Active(list)){
		Copy(list, &instr); 

		char* repl;

		printf("%s ", INSTR_STRING[instr.type]); 

		// pre každý operand inštrukcie
		for(int i = 0; i < 3; i++){
			if(instr.param[i] != NULL){
				// ak je label
				if(instr.param[i]->isLabel){
					if(instr.param[i]->type == FUNC){
						printf("$");
					}
					printf("%s ", instr.param[i]->key);
				// ak ide o konštantu
				} else if(instr.param[i]->isConst){
					// typ + príslušná hodnota
					switch(instr.param[i]->type){
						case INT: printf("int@%d ", instr.param[i]->ival); break;
						case FLOAT: printf("float@%a ", instr.param[i]->dval); break;
						case STRING: 
							repl = replace_by_escape(instr.param[i]->sval);

							if(repl == NULL){
								return ERR;
							}

							printf("string@%s ", repl);
							free(repl);
							break;
						case BOOL: printf("bool@%s ", instr.param[i]->sval); break;
						case NIL: printf("nil@%s ", instr.param[i]->sval); break;
					};
				// ináč, ak ide premennú
				} else {
					// frame
					switch(instr.param[i]->frame){
						case GF: printf("GF@"); break;
						case LF: printf("LF@"); break;
						case TF: printf("TF@"); break;
					}
					// meno premennej
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

	return 0;
}