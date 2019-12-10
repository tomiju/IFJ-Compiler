/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   parser.h
 *
 *
 * Datum:    30.11.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta    <xkucht09@stud.fit.vutbr.cz>
 */

#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "symtable.h"
#include "generator.h"

iStack indent_stack; //
TokenPTR token_ptr; //pointer na aktualni token
TokenPTR next_token; ////pointer na token po aktualnim tokenu
htab_t *globalSymtable;//globalni tabulka symbolu
htab_t *localSymtable;//lokalni tabulka symbolu
tList list;//seznam instrukci


//simulace neterminalu <paramList2>
int param();

//simulace neterminalu <paramList2>
int paramList2();

//simulace neterminalu <paramList>
int paramList();

//simulace neterminalu <funcCall>
int funcCall();

//simulace neterminalu <assignment>
int assigment();

//simulace neterminalu <statWithId>
int statWithId();

//simulace neterminalu <stat>
int stat();


//simulace neterminalu <statList>
int funcDef();


//simulace neterminalu <statList>
int statList();

//simulace neterminalu <program>
int program();

//vstupni bod parseru
int parse();

#endif

