/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   parser.h
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta    <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"
#include "symtable.h"

iStack indent_stack; //required by scanner
TokenPTR token_ptr; //pointer to the token
TokenPTR next_token;
htab_t *globalSymtable;//global table of symbols

int statList();

int param();

int paramList2();

int paramList();

int funcCall();

int assigment();

int statWithId();

int stat();

int funcDef();

int statList();

int program();

int parse();