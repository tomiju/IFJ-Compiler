#ifndef __GENERATOR_H__
#define __GENERATOR_H__

//#define str(x) #x
//#define INSTR_TO_STR(x) str(x)

/********************* LIST INŠTRUKCIÍ *********************/
typedef struct Instr{
	int type;
	void* add1;
	void* add2;
	void* add3;
} tInstr;

typedef struct Node{
	tInstr instr;
	struct Node* next;
	struct Node* prev;
} tNode;

typedef struct List{
	tNode* first;
	tNode* last;
	tNode* active;
} tList;

// Inicializuje list
void InitList(tList* list);

// Vyprázdni a uvoľní lsit
void DisposeList(tList* list);

// vloží ako prvý
int InsertFirst(tList* list, tInstr instr);

// vloží ako posledný
int InsertLast(tList* list, tInstr instr);

// nastaví aktivitu na prvý
void First(tList* list);

// nastaví aktivitu na posledný
void Last(tList* list);

// vráti hodnotu prvého
void CopyFirst(tList* list, tInstr* instr);

// vráti hodnotu posledného
void CopyLast(tList* list, tInstr* intr);

// zmaže prvý
void DeleteFirst(tList* list);

// zmaže posledný
void DeleteLast(tList* list);

// zmaže prvok za aktívnym
void PostDelete(tList* list);

// zmaže prvok pred aktívnym
void PreDelete(tList* list);

// vloží prvok za aktívnym
int PostInsert(tList* list, tInstr instr);

// zmaže prvok pred aktívnym
int PreInsert(tList* list, tInstr instr);

// vráti hodnotu 
int Copy(tList* list, tInstr* instr);

// aktualizuje hodnotu
void Actualize(tList* list, tInstr instr);

// posunie aktuálny na nasledujúci
void Succ(tList* list);

// posunie aktuálny na predchádzajúci
void Pred(tList* list);

// skontrolu aktualitu
int Active(tList* list);

/********************* LIST INŠTRUKCIÍ *********************/

/*********************** INŠTRUKCIE ************************/

typedef enum type{
	T_INT, T_FLOAT, T_DOUBLE, T_CHAR
} tType;

typedef union value {
	int ival;
	float fval;
	double dval;
	char* cval;
} tValue;

typedef struct operand{
	tType type;
	tValue val;
} tOperand;

#define INSTR(x) \
        x(MOVE)			/* <var> <symb> */				\
        x(CREATEFRAME)  								\
        x(PUSHFRAME)   									\
        x(POPFRAME)  									\
        x(DEFVAR)  		/* <var> */						\
        x(CALL)  		/* <label> */					\
        x(RETURN) 										\
        x(PUSHS)  		/* <symb> */					\
        x(POPS)  		/* <var> */						\
        x(CLEARS)  										\
        x(ADD)  		/* <var> <symb1> <symb2> */		\
        x(SUB)  		/* <var> <symb1> <symb2> */		\
        x(MUL)  		/* <var> <symb1> <symb2> */		\
        x(DIV)  		/* <var> <symb1> <symb2> */		\
        x(IDIV)  		/* <var> <symb1> <symb2> */		\
        x(ADDS)  		/* <var> <symb1> <symb2> */		\
        x(SUBS)  		/* <var> <symb1> <symb2> */		\
        x(MULS)  		/* <var> <symb1> <symb2> */		\
        x(DIVS)  		/* <var> <symb1> <symb2> */		\
        x(IDIVS)  		/* <var> <symb1> <symb2> */		\
        x(LT)  			/* <var> <symb1> <symb2> */		\
        x(GT)  			/* <var> <symb1> <symb2> */		\
        x(EQ)	  		/* <var> <symb1> <symb2> */		\
        x(LTS)  		/* <var> <symb1> <symb2> */		\
        x(GTS)  		/* <var> <symb1> <symb2> */		\
        x(EQS)  		/* <var> <symb1> <symb2> */		\
        x(AND)  		/* <var> <symb1> <symb2> */		\
        x(OR)	  		/* <var> <symb1> <symb2> */		\
        x(NOT)  		/* <var> <symb1> <symb2> */		\
        x(ADNS)  		/* <var> <symb1> <symb2> */		\
        x(ORS)  		/* <var> <symb1> <symb2> */		\
        x(NOTS)  		/* <var> <symb1> <symb2> */		\
        x(INT2FLOAT)	/* <var> <symb> */				\
		x(FLOAT2INT)	/* <var> <symb> */				\
		x(INT2CHAR)		/* <var> <symb> */				\
		x(STRI2INT)		/* <var> <symb1> <symb2> */		\
		x(INT2FLOATS)	/* <var> <symb> */				\
		x(FLOAT2INTS)	/* <var> <symb> */				\
		x(INT2CHARS)	/* <var> <symb> */				\
		x(STRI2INTS)	/* <var> <symb1> <symb2> */		\
		x(READ)			/* <var> <type> */				\
		x(WRITE)		/* <symb> */ 					\
		x(CONCAT)		/* <var> <symb1> <symb2> */		\
		x(STRLEN)		/* <var> <symb> */				\
		x(GETCHAR)		/* <var> <symb1> <symb2> */		\
		x(SETCHAR)		/* <var> <symb1> <symb2> */		\
		x(TYPE)			/* <var> <symb> */				\
		x(LABEL)		/* <label> */					\
		x(JUMP)			/* <label> */					\
		x(JUMPIFEQ)		/* <label> <symb1> <symb2> */	\
		x(JUMPIFNEQ)	/* <label> <symb1> <symb2> */	\
		x(JUMPIFEQS)	/* <label> */					\
		x(JUMPIFNEQS)	/* <label> */					\
		x(EXIT)			/* <symb> */					\
		x(BREAK)										\
		x(DPRINT)		/* <symb> */					\

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum INSTR_ENUM {
    INSTR(GENERATE_ENUM)
};

static const char* INSTR_STRING[] = {
    INSTR(GENERATE_STRING)
};

void generate_params(int count, ...);

void printInstructions(tList* list);

#endif // __GENERATOR_H__