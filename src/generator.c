#include "generator.h"
#include <stdio.h>
#include "errors.h"
#include <stdlib.h>
#include <stdarg.h>

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

/********************* LIST INŠTRUKCIÍ *********************/

/*********************** INŠTRUKCIE ************************/

/*void function_start(){
	//LABEL $foo
	//P11USHFRAME 

	// definícia premenných

	// code for function

	// POPFRAME
	// RETURN
}*/

#define GENERATE_PARAM(x, param) \
	do{ \
		\
		\
		\
	} while(0)\

void generate_params(int count, ...){
	va_list list;

	va_start(list, count);
	for(int i = 0; i < count; i++){
		GENERATE_PARAM(i, va_arg(list, struct operand));
	}

	va_end(list);
}

void printInstructions(tList* list){
	printf(".IFJcode19\n");

	First(list);
	tInstr instr;

	while(Active(list)){
		Copy(list, &instr); 
		printf("%s\n", INSTR_STRING[instr.type]);
	}
}