/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   main_parser.c
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta    <xkucht09@stud.fit.vutbr.cz>
 */

#include <stdio.h>
#include "scanner.h"
#include "parser.h"

int main(int argc, char const *argv[]){
    FILE *f;
    
    if (argc == 1) // TODO: do jedné funkce
    {
        fprintf(stderr,"No input file.\n");
        return -1;
    }
    if ((f = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr,"File couldn't be opened.\n");
        return -1;
    }

    setSourceFile(f);

    int result =  parse();

    fclose(f);

    printf("result: %d\n",result);
    return result;
}
    